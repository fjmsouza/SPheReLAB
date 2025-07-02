#include "main_aux.h"
#include "mobilenetv3.h" 
// #include "mobilenetv3small_hybrid.h"

// --- TFLite includes: C++ headers, NÃO podem estar em extern "C"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <esp_heap_caps.h> // heap_caps_
#include <esp_system.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_psram.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "esp_flash.h"
#include "esp_private/esp_clk.h"
#include "esp_idf_version.h"


// Parâmetros do modelo mobilenetv3 reduzido
constexpr int IMG_H = 224;
constexpr int IMG_W = 224;
constexpr int IMG_C = 3;
// Defina o tamanho do tensor arena conforme necessário (experimente aumentar/diminuir se faltar RAM)
constexpr int kTensorArenaSize = 1024 * 1024;

// -- Configuração dos pinos da câmera --
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 10
#define SIOD_GPIO_NUM 40
#define SIOC_GPIO_NUM 39
#define Y9_GPIO_NUM 48
#define Y8_GPIO_NUM 11
#define Y7_GPIO_NUM 12
#define Y6_GPIO_NUM 14
#define Y5_GPIO_NUM 16
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 17
#define Y2_GPIO_NUM 15
#define VSYNC_GPIO_NUM 38
#define HREF_GPIO_NUM 47
#define PCLK_GPIO_NUM 13

// esp_task_wdt_config_t wdt_config = {
//     .timeout_ms = 10000,                             // 10 segundos (em milissegundos!)
//     .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // Para ambos os cores, se quiser
//     .trigger_panic = true                            // Se deve fazer panic (reset) ao estourar
// };

void setup_camera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 63;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // Inicializa câmera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        ESP_LOGE("cam", "Erro ao iniciar camera: %s", esp_err_to_name(err));
        while (1)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

void setup_psram()
{
    if (!esp_psram_is_initialized())
    {
        ESP_LOGE("main", "PSRAM não encontrada");
        return;
    }

    ESP_LOGE("main", "PSRAM encontrada: %dMB", esp_psram_get_size() / 1048576);
}

void report(unsigned long elapsed)
{
    // SRAM interna (heap DRAM)
    size_t sram_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    // PSRAM externa
    size_t psram_total = esp_psram_get_size();

    // Flash
    uint32_t flash_size = 0;
    esp_flash_get_size(esp_flash_default_chip, &flash_size);

    // Clock
    unsigned int freq_mhz = (unsigned int)(esp_clk_cpu_freq() / 1000000U);

    printf("===== RELATÓRIO DO HARDWARE =====\n");
    printf("ESP-IDF version: %d.%d.%d\n", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);

    printf("SRAM total (heap interna):   %u bytes (%.2f KB)\n", (unsigned int)sram_total, sram_total/1024.0);
    printf("PSRAM total:                 %u bytes (%.2f MB)\n", (unsigned int)psram_total, psram_total/1048576.0);
    printf("Tamanho da Flash:            %u bytes (%.2f MB)\n", (unsigned int)flash_size, flash_size/1048576.0);
    printf("Frequência do clock da CPU:  %u MHz\n", freq_mhz);
    printf("=================================\n");
}

extern "C" void app_main(void)
{
    setup_psram();
    setup_camera();

    uint8_t *tensor_arena = (uint8_t *)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);
    // Verifica se a alocação foi bem-sucedida
    if (!tensor_arena)
    {
        ESP_LOGE("main", "Falha ao alocar tensor_arena na PSRAM!");
        return;
    }

    // Carregar o modelo TFLite em RAM
    const tflite::Model *model = tflite::GetModel(g_model); // array gerado a partir do seu .tflite

    // O número abaixo deve ser igual ou maior que o número de Add* chamados
    static tflite::MicroMutableOpResolver<9> resolver;

    resolver.AddAdd();
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddFullyConnected();
    resolver.AddHardSwish();
    resolver.AddMean();
    resolver.AddMul();
    resolver.AddSoftmax();

    static tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, kTensorArenaSize);

    TfLiteStatus allocate_status = interpreter.AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        ESP_LOGE("main", "Erro ao alocar tensores!");
        // (opcional: printa quanto de memória tem disponível)
        return;
    }

    // Só agora pode acessar:
    TfLiteTensor *input = interpreter.input(0);

    while (true)
    {
        unsigned long start_time = esp_timer_get_time() / 1000; // em milissegundos

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE("main", "Erro ao capturar frame");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        // ESP_LOGE("main", "Frame capturado: %d bytes\n", fb->len);

        // -- Processamento: recorte customizado
        uint8_t *cropped_leaves_bgr = cut_leaves_without_spark((uint8_t *)fb->buf, fb->width, fb->height);

        // -- Redimensionar imagem para 224x224
        // Aloca o buffer para a imagem redimensionada na PSRAM
        uint8_t *img_resized = (uint8_t *)heap_caps_malloc(IMG_H * IMG_W * IMG_C, MALLOC_CAP_SPIRAM);
        if (!img_resized)
        {
            ESP_LOGE("main", "Falha ao alocar img_resized!");
            if (cropped_leaves_bgr != (uint8_t *)fb->buf)
                free(cropped_leaves_bgr);
            esp_camera_fb_return(fb);
            continue;
        }
        resize_bgr_nearest(cropped_leaves_bgr, fb->width, fb->height, img_resized, IMG_W, IMG_H);
        // -- Converter para float32 entre 0-1 (ou int8, conforme seu modelo)
        for (int i = 0; i < IMG_H * IMG_W * IMG_C; i++)
        {
            input->data.f[i] = img_resized[i] / 255.0f;
        }

        // Inferência
        if (interpreter.Invoke() != kTfLiteOk)
        {
            ESP_LOGE("main", "Erro ao executar modelo");
        }

        TfLiteTensor *output = interpreter.output(0);

        int pred = -1;
        float best = -1;
        for (int i = 0; i < output->dims->data[1]; ++i)
        {
            if (output->data.f[i] > best)
            {
                best = output->data.f[i];
                pred = i;
            }
        }
        ESP_LOGI("main", "Classe prevista: %d, confiança: %.2f", pred, best);

        unsigned long end_time = esp_timer_get_time() / 1000; // em milissegundos
        unsigned long elapsed = end_time - start_time;
        ESP_LOGI("main", "Tempo de Pipeline: %lu ms", elapsed);

        // Libera buffers na ordem correta:
        free(img_resized);
        if (cropped_leaves_bgr != (uint8_t *)fb->buf)
            free(cropped_leaves_bgr); // Só libera se realmente foi alocado
        esp_camera_fb_return(fb);

        report(elapsed);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
