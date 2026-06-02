#include "drivers.h"
#include "includes.h"

#define SENSOR420MA 0

extern SemaphoreHandle_t xSerialMutex;


void initADC()
{
    analogReadResolution(12);
#if SENSOR420MA
    analogSetAttenuation(A1,ADC_11db);
#endif
    pinMode(ADC_PIN, INPUT);
}

uint16_t readADC()
{
    return analogRead(ADC_PIN);
}


uint16_t readADCavg(uint8_t ADC_PIN_NUM)
{
    uint32_t sum = 0;

    for (int i = 0; i < 64; i++)
    {
        sum += analogRead(ADC_PIN_NUM);
        delayMicroseconds(200);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return sum / 64;
}
    


/*
// A1 is GPIO1 on XIAO ESP32-S3
#define SENSOR_PIN A1

void analogReadTask(void *pvParameters) {
    while (true) {
        int raw = analogRead(SENSOR_PIN);      // 0–4095 (12-bit ADC)
        float voltage = raw * (3.3f / 4095.0f);

        Serial.printf("A1 raw: %d  voltage: %.2fV\n", raw, voltage);

        vTaskDelay(pdMS_TO_TICKS(500));        // yield to other tasks
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // wait for USB CDC to connect

    analogReadResolution(12);  // ESP32-S3 supports 12-bit (default)
    pinMode(SENSOR_PIN, INPUT);

    // Create the task on Core 1 (Core 0 is used by WiFi/BT)
    xTaskCreatePinnedToCore(
        analogReadTask,   // function
        "AnalogRead",     // name
        2048,             // stack size (bytes)
        NULL,             // parameter
        1,                // priority
        NULL,             // task handle
        1                 // core (0 or 1)
    );
}

*/

