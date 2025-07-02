#include <Arduino.h>
#include "esp_camera.h"
#include <math.h>
#include <esp_heap_caps.h> // Para funções heap_caps_
#include <esp32/spiram.h>  // Para esp_spiram_get_size
#include <esp_system.h>    // Para esp_get_free_heap_size, etc.
#include <stdio.h>         // Para printf
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "decision_tree.h" // Inclua o cabeçalho da sua árvore de decisão


struct Pixel
{
    uint8_t b, g, r;
};

struct HSV
{
    float h, s, v;
};

using namespace std;
const int GRAY_LEVELS = 256;

// Matriz GLCM
float glcm[GRAY_LEVELS][GRAY_LEVELS] = {0};
float contrast, energy, homogeneity, correlation, std_dev, entropy;

// Declaração da função de inferência (externa, C)

    // void score(double *input, double *output);

// Pinos da câmera do seu módulo — verifique com seu modelo
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

camera_fb_t *img1 = nullptr;

// Converte BGR para HSV
HSV bgrToHsv(uint8_t b, uint8_t g, uint8_t r)
{
    float r_f = r / 255.0f, g_f = g / 255.0f, b_f = b / 255.0f;
    float cmax = fmaxf(r_f, fmaxf(g_f, b_f));
    float cmin = fminf(r_f, fminf(g_f, b_f));
    float delta = cmax - cmin;

    float h = 0;
    if (delta != 0)
    {
        if (cmax == r_f)
            h = 60 * fmodf(((g_f - b_f) / delta), 6);
        else if (cmax == g_f)
            h = 60 * (((b_f - r_f) / delta) + 2);
        else
            h = 60 * (((r_f - g_f) / delta) + 4);
    }
    if (h < 0)
        h += 360;

    float s = (cmax == 0) ? 0 : (delta / cmax);
    float v = cmax;

    HSV hsv = {h, s, v};
    return hsv;
}

// Converte BGR para grayscale (tons de cinza)
uint8_t *bgr_to_gray(const uint8_t *bgr, int width, int height)
{
    int size = width * height;
    uint8_t *gray = (uint8_t *)malloc(size);
    if (!gray)
    {
        Serial.println("Erro ao alocar memória para imagem em tons de cinza");
        return nullptr;
    }

    for (int i = 0; i < size; ++i)
    {
        int idx = i * 3;
        uint8_t b = bgr[idx + 0];
        uint8_t g = bgr[idx + 1];
        uint8_t r = bgr[idx + 2];
        gray[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
    }

    return gray; // lembre-se de liberar com free() depois
}

// Função principal: processa a imagem e retorna novo buffer BGR processado
uint8_t *cut_leaves_without_spark(uint8_t *input_bgr, int width, int height)
{
    int size = width * height * 3;

    uint8_t *output_bgr = (uint8_t *)malloc(size);
    if (!output_bgr)
    {
        Serial.println("Erro ao alocar output_bgr");
        return nullptr;
    }

    // 1) Calcula média do canal V na região referência (spark)
    int ref_x = 0, ref_y = 0, lado = 10;
    float soma_v = 0;
    int count = 0;
    for (int y = ref_y; y < ref_y + lado && y < height; ++y)
    {
        for (int x = ref_x; x < ref_x + lado && x < width; ++x)
        {
            int idx = (y * width + x) * 3;
            HSV hsv = bgrToHsv(input_bgr[idx], input_bgr[idx + 1], input_bgr[idx + 2]);
            soma_v += hsv.v * 255;
            ++count;
        }
    }
    float ref_v_mean = (count > 0) ? (soma_v / count) : 0;

    // 2) Remove brilho (spark) da imagem original
    for (int i = 0; i < size; i++)
    {
        float val = (float)input_bgr[i] - ref_v_mean;
        output_bgr[i] = (val < 0) ? 0 : (uint8_t)val;
    }

    // 4) Segmentação por cor (verde, marrom, branco)
    uint8_t *mask = (uint8_t *)calloc(width * height, 1);
    if (!mask)
    {
        Serial.println("Erro ao alocar mask");
        free(output_bgr);
        return nullptr;
    }

    for (int i = 0; i < width * height; ++i)
    {
        HSV hsv = bgrToHsv(output_bgr[i * 3], output_bgr[i * 3 + 1], output_bgr[i * 3 + 2]);
        float h = hsv.h, s = hsv.s * 255, v = hsv.v * 255;

        bool green = (h >= 32 && h <= 60);
        bool brown = (h >= 22 && h <= 35) && (s >= 63) && (v >= 20 && v <= 200);
        bool white = (h == 0 && s == 0); // branco saturado

        if (green || brown || white)
            mask[i] = 255;
    }

    // 5) Converter BGR com máscara para grayscale usando bgr_to_gray e aplicar máscara
    uint8_t *gray_full = bgr_to_gray(output_bgr, width, height);
    if (!gray_full)
    {
        Serial.println("Erro ao alocar gray_full");
        free(mask);
        free(output_bgr);
        printf("resetando a esp");
        esp_restart(); // Reinicia a ESP32 se falhar ao capturar imagem
        // return nullptr;
    }

    uint8_t *gray = (uint8_t *)malloc(width * height);
    if (!gray)
    {
        Serial.println("Erro ao alocar gray");
        free(gray_full);
        free(mask);
        free(output_bgr);
        printf("resetando a esp");
        esp_restart(); // Reinicia a ESP32 se falhar ao capturar imagem
        // return nullptr;
    }
    for (int i = 0; i < width * height; ++i)
        gray[i] = mask[i] ? gray_full[i] : 0;
    free(gray_full);

    // 7) Threshold: Otsu
    uint8_t threshold = 0;
    if (threshold == 0)
    {
        // Otsu simplificado
        uint16_t hist[256] = {0};
        for (int i = 0; i < width * height; ++i)
            hist[gray[i]]++;

        int total = width * height;
        float sum = 0;
        for (int i = 0; i < 256; ++i)
            sum += i * hist[i];

        float sumB = 0, wB = 0, maxVar = 0;
        for (int t = 0; t < 256; ++t)
        {
            wB += hist[t];
            if (wB == 0)
                continue;
            float wF = total - wB;
            if (wF == 0)
                break;

            sumB += t * hist[t];
            float mB = sumB / wB;
            float mF = (sum - sumB) / wF;
            float varBetween = wB * wF * (mB - mF) * (mB - mF);

            if (varBetween > maxVar)
            {
                maxVar = varBetween;
                threshold = t;
            }
        }
    }

    // 8) Aplicar limiar (threshold) para binarizar
    uint8_t *bin = (uint8_t *)malloc(width * height);
    if (!bin)
    {
        Serial.println("Erro ao alocar bin");
        free(gray);
        free(mask);
        free(output_bgr);
        return nullptr;
    }
    for (int i = 0; i < width * height; ++i)
        bin[i] = (gray[i] > threshold) ? 255 : 0;
    free(gray);

    // 9) Aplicar filtro mediana 3x3 simples (janela)
    uint8_t *mask_final = (uint8_t *)malloc(width * height);
    if (!mask_final)
    {
        Serial.println("Erro ao alocar mask_final");
        free(bin);
        free(mask);
        free(output_bgr);
        printf("resetando a esp");
        esp_restart(); // Reinicia a ESP32 se falhar ao capturar imagem
        return nullptr;
    }

    for (int y = 1; y < height - 1; ++y)
    {
        for (int x = 1; x < width - 1; ++x)
        {
            uint8_t window[9];
            int k = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    window[k++] = bin[(y + dy) * width + (x + dx)];

            // ordena janela 3x3
            for (int i = 0; i < 8; ++i)
            {
                for (int j = i + 1; j < 9; ++j)
                {
                    if (window[j] < window[i])
                    {
                        uint8_t temp = window[i];
                        window[i] = window[j];
                        window[j] = temp;
                    }
                }
            }
            mask_final[y * width + x] = window[4]; // mediana
        }
    }
    // bordas 1px zeradas
    for (int x = 0; x < width; ++x)
    {
        mask_final[x] = 0;
        mask_final[(height - 1) * width + x] = 0;
    }
    for (int y = 0; y < height; ++y)
    {
        mask_final[y * width] = 0;
        mask_final[y * width + (width - 1)] = 0;
    }
    free(bin);
    free(mask);

    // 10) Aplica máscara final para apagar pixels fora da máscara (deixa preto)
    for (int i = 0; i < width * height; ++i)
    {
        if (mask_final[i] == 0)
        {
            output_bgr[i * 3 + 0] = 0;
            output_bgr[i * 3 + 1] = 0;
            output_bgr[i * 3 + 2] = 0;
        }
    }
    free(mask_final);

    return output_bgr;
}

// Função para gerar a GLCM com deslocamento (1, 0) — direção horizontal
void computeGLCM(uint8_t *image, int width, int height)
{
    // Zera a matriz GLCM (importante para evitar acumular entre frames)
    for (int i = 0; i < GRAY_LEVELS; i++)
    {
        for (int j = 0; j < GRAY_LEVELS; j++)
        {
            glcm[i][j] = 0.0;
        }
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            uint8_t i = image[y * width + x];
            uint8_t j = image[y * width + (x + 1)];
            if (i < GRAY_LEVELS && j < GRAY_LEVELS)
            {
                glcm[i][j] += 1.0;
            }
        }
    }

    // Normaliza a GLCM
    float sum = 0.0;
    for (int i = 0; i < GRAY_LEVELS; i++)
    {
        for (int j = 0; j < GRAY_LEVELS; j++)
        {
            sum += glcm[i][j];
        }
    }

    for (int i = 0; i < GRAY_LEVELS; i++)
    {
        for (int j = 0; j < GRAY_LEVELS; j++)
        {
            glcm[i][j] /= sum;
        }
    }
}

// Função para calcular contraste, energia, homogeneidade, correlação, desvio padrão e entropia
void computeGLCMFeatures(
    float &contrast, float &energy, float &homogeneity,
    float &correlation, float &std_dev, float &entropy)
{
    contrast = 0.0;
    energy = 0.0;
    homogeneity = 0.0;
    correlation = 0.0;
    entropy = 0.0;
    std_dev = 0.0;

    float mean_i = 0, mean_j = 0;
    float std_i = 0, std_j = 0;

    // Primeira passada: médias
    for (int i = 0; i < GRAY_LEVELS; i++)
    {
        for (int j = 0; j < GRAY_LEVELS; j++)
        {
            float p = glcm[i][j];
            mean_i += i * p;
            mean_j += j * p;
        }
    }

    // Segunda passada: desvios padrão
    for (int i = 0; i < GRAY_LEVELS; i++)
    {
        for (int j = 0; j < GRAY_LEVELS; j++)
        {
            float p = glcm[i][j];
            std_i += (i - mean_i) * (i - mean_i) * p;
            std_j += (j - mean_j) * (j - mean_j) * p;
        }
    }

    std_i = sqrt(std_i);
    std_j = sqrt(std_j);
    std_dev = (std_i + std_j) / 2.0; // média dos desvios como uma única medida

    // Terceira passada: medidas principais
    for (int i = 0; i < GRAY_LEVELS; i++)
    {
        for (int j = 0; j < GRAY_LEVELS; j++)
        {
            float p = glcm[i][j];
            if (p > 0.0f)
            {
                entropy += -p * log2(p); // Evita log(0)
            }
            contrast += (i - j) * (i - j) * p;
            energy += p * p;
            homogeneity += p / (1.0f + abs(i - j));
            if (std_i > 0.0f && std_j > 0.0f)
            {
                correlation += ((i - mean_i) * (j - mean_j) * p) / (std_i * std_j);
            }
        }
    }
}

void myHeavyTask(void *param)
{
    double input[6] = {0.3, 0.2, 0.5, 0.6, 0.4, 0.7};
    double output[2];

    while (true)
    {

        unsigned long start_time = millis();

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Falha ao capturar imagem");
            delay(1000);
            printf("resetando a esp");
            esp_restart(); // Reinicia a ESP32 se falhar ao capturar imagem
        }
        int w = fb->width;
        int h = fb->height;

        // Processa a imagem capturada para remover brilho, fundo, etc.
        uint8_t *processed = cut_leaves_without_spark(fb->buf, w, h);
        if (!processed)
        {
            Serial.println("Erro ao processar a imagem");
            esp_camera_fb_return(fb);
            printf("resetando a esp");
            esp_restart(); // Reinicia a ESP32 se falhar ao capturar imagem
        }

        computeGLCM(processed, w, h);
        computeGLCMFeatures(contrast, energy, homogeneity, correlation, std_dev, entropy);

        // Prepara input e chama a árvore de decisão
        input[0] = contrast;
        input[1] = energy;
        input[2] = homogeneity;
        input[3] = correlation;
        input[4] = std_dev;
        input[5] = entropy;

        // Chamar função da árvore de decisão
        score(input, output);

        unsigned long end_time = millis();
        unsigned long elapsed = end_time - start_time;


        Serial.printf("Tempo de captura, processamento e inferência: %lu ms\n", elapsed);

        vTaskDelay(pdMS_TO_TICKS(2000));
 
        // Libera as imagens
        esp_camera_fb_return(fb);
        free(processed);
        free(fb);

        // Reinicia a ESP32 após cada iteração
        Serial.println("Aguardando 1 segundo antes de reiniciar...");
        delay(1000);
        esp_restart(); // Reinicia a ESP32 após cada iteração
    }
}

void setup()
{
    delay(5000); // Wait for serial to initialize
    Serial.begin(115200);
    Serial.printf("PSRAM: %dMB\n", esp_spiram_get_size() / 1048576);
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
    config.frame_size = FRAMESIZE_XGA; // FRAMESIZE_WQXGA;
    config.jpeg_quality = 63;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

    if (!psramFound())
    {
        Serial.println("PSRAM não encontrada");
        return;
    }

    if (esp_camera_init(&config) != ESP_OK)
    {
        Serial.println("Erro ao inicializar a câmera");
        return;
    }

    Serial.println("Câmera iniciada com sucesso");
    xTaskCreatePinnedToCore(
        myHeavyTask,
        "myHeavyTask",
        16384, // stack size em palavras (8192 * 4 = 32 KB)
        NULL,
        1,
        NULL,
        1 // núcleo 1 geralmente é o que roda o Arduino loop()
    );
}

void loop()
{
}