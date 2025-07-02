#pragma once

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"  // Add this include for rtc_gpio functions
#include "Arduino.h"
#include "esp_camera.h"



// Forward declarations
class ConnectionHandler;
class CameraHandler;

// Definições e constantes
#define DRY 338
#define SOAKED 128
#define uS_TO_S_FACTOR 1000000ULL
#define PUMP_MAXIMUM_PERIOD 60000
#define PUMP_MINIMUM_PERIOD 5000
#define MOISTURE_SENSOR 1
#define PUMP 7

// Estrutura para os limiares de histerese
struct Hysteresis
{
    int upper_threshold;
    int lower_threshold;
};

// Declaração das variáveis globais
extern struct Hysteresis thresholds;
// Constantes
constexpr int SAMPLES_EFFECTIVE_NUMBER = 256;
constexpr int SAMPLES_TOTAL_NUMBER = SAMPLES_EFFECTIVE_NUMBER + 2;

// Declarações "extern" (definições em system.cpp)
extern String water_command;
extern unsigned long sleep_period;
extern unsigned long sleep_period_aux1;
extern unsigned long SLEEP_PERIOD;
extern bool turn_on;
extern int moisture;
extern int failure_count;


enum State
{
    moisture_read,
    send_image,
    pump_control,
    send_data,
    deep_sleep
};

extern enum State state;

// Protótipos das funções
void setupPinout();
void pumpControl(bool flag, int pump_on_period);
int moistureRead();
void updateCommandThresholds();
void systemPowerOff();
void handleStates();