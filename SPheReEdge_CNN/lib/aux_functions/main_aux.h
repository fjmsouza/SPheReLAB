#ifndef MAIN_AUX_H
#define MAIN_AUX_H

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h" // esp_restart


// #ifdef __cplusplus
// extern "C" {
// #endif

struct Pixel {
    uint8_t b, g, r;
};

struct HSV {
    float h, s, v;
};

void resize_bgr_nearest(const uint8_t* src, int src_w, int src_h, uint8_t* dst, int dst_w, int dst_h);
struct HSV bgr_to_hsv(uint8_t b, uint8_t g, uint8_t r);
uint8_t* bgr_to_gray(const uint8_t* bgr, int width, int height);
uint8_t* cut_leaves_without_spark(uint8_t* input_bgr, int width, int height);

// #ifdef __cplusplus
// }
// #endif

#endif // MAIN_AUX_H
