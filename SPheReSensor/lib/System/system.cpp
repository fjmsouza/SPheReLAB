#include "system.h"
#include "Connection.h"
#include "Camera.h"
#include "Storage.h"

// Definições (com inicialização)
struct Hysteresis thresholds;
String water_command = "";
enum State state = moisture_read;
int moisture = 0;
bool turn_on = false;
unsigned long sleep_period = 15;
unsigned long sleep_period_aux1 = sleep_period * 60;
unsigned long SLEEP_PERIOD = sleep_period_aux1 * uS_TO_S_FACTOR;
RTC_DATA_ATTR int failure_count = 0;

// Definições de pinos
void setupPinout()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    pinMode(PUMP, OUTPUT); // GPIO for LED flash
    digitalWrite(PUMP, LOW);
    rtc_gpio_hold_dis(GPIO_NUM_4); // disable pin hold if it was enabled before sleeping
}

// Função que controla a bomba
void pumpControl(bool flag, int pump_on_period)
{

    if (flag)
    {
        delay(1000);
        digitalWrite(PUMP, HIGH);
        Serial.printf("Pump turned on! Wait %d secs", pump_on_period / 1000);
        delay(pump_on_period);
        digitalWrite(PUMP, LOW);
        Serial.println("Pump turned off!");
    }
    else
    {
        digitalWrite(PUMP, LOW);
        Serial.println("Pump turned off!");
    }
}

// Função que lê o sensor de umidade e processa os valores
int moistureRead(int moisture_sensor)
{
    int moisture = 0;
    int sum = 0;
    int current = 0;
    int array_samples[SAMPLES_TOTAL_NUMBER];

    for (int m = 0; m < SAMPLES_TOTAL_NUMBER; m++)
    {
        current = analogRead(moisture_sensor);
        array_samples[m] = current;
        sum += current;
    }

    int minValue = min(array_samples[0], array_samples[1]);
    int maxValue = max(array_samples[0], array_samples[1]);

    for (int i = 2; i < SAMPLES_TOTAL_NUMBER; i++)
    {
        minValue = min(minValue, array_samples[i]);
        maxValue = max(maxValue, array_samples[i]);
    }

    moisture = (sum - minValue - maxValue) / SAMPLES_EFFECTIVE_NUMBER;

    if (moisture <= SOAKED)
    {
        moisture = SOAKED;
    }
    if (moisture >= DRY)
    {
        moisture = DRY;
    }

    // Mapeia o valor de umidade para uma escala de 0 a 100%
    moisture = map(moisture, DRY, SOAKED, 0, 100);
    return moisture;
}

// Função para atualizar os limiares de histerese usando a Storage e dados recebidos
void updateCommandThresholds()
{
    if (Connection.connection_status)
    {
        if ((Storage.fileExists(Storage.UPPER_THRESHOLD_PATH)) && (Storage.fileExists(Storage.LOWER_THRESHOLD_PATH)))
        {
            // Os arquivos já existem
        }
        else
        {
            Storage.createFile(Storage.UPPER_THRESHOLD_PATH);
            Storage.createFile(Storage.LOWER_THRESHOLD_PATH);
            Storage.writeString(Storage.UPPER_THRESHOLD_PATH, "00");
            Storage.writeString(Storage.LOWER_THRESHOLD_PATH, "00");
        }

        String data = Connection.receiveData();
        int index1 = data.indexOf(',');
        int index2 = data.indexOf(',', index1 + 1);

        water_command = data.substring(0, index1);
        String upper_threshold_received = data.substring(index1 + 1, index2);
        String lower_threshold_received = data.substring(index2 + 1);

        String upper_threshold_recorded = Storage.readString(Storage.UPPER_THRESHOLD_PATH);
        String lower_threshold_recorded = Storage.readString(Storage.LOWER_THRESHOLD_PATH);

        if ((upper_threshold_received != upper_threshold_recorded) || (lower_threshold_received != lower_threshold_recorded))
        {
            if ((upper_threshold_received.toInt() >= 0) && (lower_threshold_received.toInt() >= 0) && (upper_threshold_received.toInt() >= lower_threshold_received.toInt()))
            {
                Storage.writeString(Storage.UPPER_THRESHOLD_PATH, upper_threshold_received);
                Storage.writeString(Storage.LOWER_THRESHOLD_PATH, lower_threshold_received);
            }
        }
    }
    else
    {
        if ((Storage.fileExists(Storage.UPPER_THRESHOLD_PATH)) && (Storage.fileExists(Storage.LOWER_THRESHOLD_PATH)))
        {
            // Já existem
        }
        else
        {
            Storage.createFile(Storage.UPPER_THRESHOLD_PATH);
            Storage.createFile(Storage.LOWER_THRESHOLD_PATH);
            Storage.writeString(Storage.UPPER_THRESHOLD_PATH, "00");
            Storage.writeString(Storage.LOWER_THRESHOLD_PATH, "00");
        }
    }
    thresholds.upper_threshold = Storage.readString(Storage.UPPER_THRESHOLD_PATH).toInt();
    thresholds.lower_threshold = Storage.readString(Storage.LOWER_THRESHOLD_PATH).toInt();

    Serial.println("valores dos limiares:");
    Serial.println(thresholds.upper_threshold);
    Serial.println(thresholds.lower_threshold);

    if (water_command != "water")
    {
        water_command = "nowater";
    }
}

void systemPowerOff()
{
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(PUMP, LOW);
    rtc_gpio_hold_en(GPIO_NUM_7); // make sure pump is held LOW in sleep
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
}

// Função que gerencia os estados do sistema
void handleStates()
{
    digitalWrite(LED_BUILTIN, LOW); // Garante que o LED esteja apagado

    switch (state)
    {
    case moisture_read:
        Serial.println("\nmoisture_read");
        moisture = moistureRead(MOISTURE_SENSOR);
        Serial.printf("\nmoisture = %d \n", moisture);
        if (Connection.connection_status)
        {
            state = send_image;
        }
        else
        {
            state = pump_control;
        }
        break;

    case send_image:
        Serial.println("send_image");

        // Captura e envia imagem se ambiente iluminado
        image = Camera.takeDayPicture();
        if (image)
        {
            Serial.printf("JPEG size: %d bytes\n", image->len);
            Connection.sendImage(moisture, image);
        }
        Camera.powerOff();
        state = pump_control;
        break;

    case pump_control:
        Serial.println("pump_control");

        updateCommandThresholds();

        if (water_command == "water")
        {
            turn_on = true;
            pumpControl(turn_on, PUMP_MAXIMUM_PERIOD);
        }
        else
        {
            if (moisture < thresholds.lower_threshold)
            {
                turn_on = true;
            }
            else if (moisture >= thresholds.upper_threshold)
            {
                turn_on = false;
            }
            pumpControl(turn_on, PUMP_MINIMUM_PERIOD);
        }
        

        if (Connection.connection_status)
        {
            state = send_data;
        }
        else
        {
            state = deep_sleep;
        }
        break;
    case send_data:
        Serial.println("send_data");
        Connection.sendData(moisture, turn_on);
        state = deep_sleep;
        break;
    case deep_sleep:
        Serial.println("deep_sleep");
        
        if (failure_count > 3)
        {
            Serial.println("Failure count FULL = " + String(failure_count));
            failure_count = 0;
            ESP.restart();
        }
        else
        {
            Serial.println("Failure count not full yet  = " + String(failure_count));
        }
        systemPowerOff();
        break;

    default:
        break;
    }
}
