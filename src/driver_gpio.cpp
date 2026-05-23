#include "drivers.h"
#include "includes.h"

#define DEBUG_GPIO 0


void hwTaskLED(void *pvParameters) 
{
   TickType_t xLastWakeTime = xTaskGetTickCount();
   const TickType_t xPeriod = pdMS_TO_TICKS(500);
    // On Board LED heatbeat
    pinMode(BeatLed, OUTPUT);

   while(1) {  
      digitalWrite(BeatLed, !digitalRead(BeatLed));
      vTaskDelayUntil(&xLastWakeTime, xPeriod);        
   }
}


void initFwRevision() 
{
  logf("\n");
  logf("*************** IoT Device ************** \n");  
  logf("        Firmware Rev: %c \n", String(fwVersions[0]));
  logf("        Portal Rev: %c \n", String(fwVersions[1]));
  logf("*************************************** \n");
}

