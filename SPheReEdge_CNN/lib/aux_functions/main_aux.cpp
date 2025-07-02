#include "main_aux.h"

void resize_bgr_nearest(
    const uint8_t *src, int src_w, int src_h,
    uint8_t *dst, int dst_w, int dst_h)
{
    for (int y = 0; y < dst_h; ++y)
    {
        int src_y = y * src_h / dst_h;
        for (int x = 0; x < dst_w; ++x)
        {
            int src_x = x * src_w / dst_w;
            for (int c = 0; c < 3; ++c)
            {
                dst[(y * dst_w + x) * 3 + c] =
                    src[(src_y * src_w + src_x) * 3 + c];
            }
        }
    }
}

// Converte BGR para HSV
struct HSV bgr_to_hsv(uint8_t b, uint8_t g, uint8_t r)
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

    struct HSV hsv = {h, s, v};
    return hsv;
}

uint8_t *bgr_to_gray(const uint8_t *bgr, int width, int height)
{
    int size = width * height;
    uint8_t *gray = (uint8_t *)heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (!gray)
    {
        ESP_LOGE("main_aux", "Erro ao alocar memória para imagem em tons de cinza");
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

    uint8_t *output_bgr = (uint8_t *)heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (!output_bgr)
    {
        ESP_LOGE("main_aux", "Erro ao alocar output_bgr");
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
            HSV hsv = bgr_to_hsv(input_bgr[idx], input_bgr[idx + 1], input_bgr[idx + 2]);
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
    uint8_t *mask = (uint8_t *)heap_caps_malloc(width * height, MALLOC_CAP_SPIRAM);
    if (!mask)
    {
        ESP_LOGE("main_aux", "Erro ao alocar mask");
        free(output_bgr); // <-- LIBERA output_bgr caso falhe aqui
        return nullptr;
    }

    for (int i = 0; i < width * height; ++i)
    {
        HSV hsv = bgr_to_hsv(output_bgr[i * 3], output_bgr[i * 3 + 1], output_bgr[i * 3 + 2]);
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
        ESP_LOGE("main_aux", "Erro ao alocar gray_full");
        free(mask);       // <-- LIBERA mask
        free(output_bgr); // <-- LIBERA output_bgr
        return nullptr;
    }

    uint8_t *gray = (uint8_t *)heap_caps_malloc(width * height, MALLOC_CAP_SPIRAM);
    if (!gray)
    {
        ESP_LOGE("main_aux", "Erro ao alocar gray");
        free(gray_full);  // <-- LIBERA gray_full
        free(mask);       // <-- LIBERA mask
        free(output_bgr); // <-- LIBERA output_bgr
        return nullptr;
    }
    for (int i = 0; i < width * height; ++i)
        gray[i] = mask[i] ? gray_full[i] : 0;

    free(mask); // <-- LIBERA mask
    free(gray_full);

    // 7) Threshold: Otsu
    uint8_t threshold;

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

    // 8) Aplicar limiar (threshold) para binarizar
    uint8_t *bin = (uint8_t *)heap_caps_malloc(width * height,MALLOC_CAP_SPIRAM);
    if (!bin)
    {
        ESP_LOGE("main_aux", "Erro ao alocar bin");
        free(gray);       // <-- LIBERA gray
        free(gray_full);  // <-- LIBERA gray_full
        free(mask);       // <-- LIBERA mask
        free(output_bgr); // <-- LIBERA output_bgr
        return nullptr;
    }
    for (int i = 0; i < width * height; ++i)
        bin[i] = (gray[i] > threshold) ? 255 : 0;
    free(gray);

    // 9) Aplicar filtro mediana 3x3 simples (janela)
    uint8_t *mask_final = (uint8_t *)heap_caps_malloc(width * height,MALLOC_CAP_SPIRAM);
    if (!mask_final)
    {
        ESP_LOGE("main_aux", "Erro ao alocar mask_final");
        free(bin);        // <-- LIBERA bin
        free(gray_full);  // <-- LIBERA gray_full
        free(mask);       // <-- LIBERA mask
        free(output_bgr); // <-- LIBERA output_bgr
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
    free(bin);

    return output_bgr;
}