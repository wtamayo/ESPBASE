/***************************************************************************************
 * 
 * Multi-task communication hub on ESP32:
 *  
 * Each interface runs in its own task:
 *  RS232 → vTaskRS232
 *  UDP → vTaskUDP
 *  CAN → vTaskCAN
 *  
 *  All tasks send messages to a central mailbox (queue)
 *  A dispatcher task (vPrintTsk) receives and processes/logs messages
 *  This is a producer → queue → consumer pattern
 * 
 ***************************************************************************************
 */
#include "includes.h"
#include "utils.h"
#include "drivers.h"
#include "networking.h"
#include "wbserver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "freertos/task.h"

#define DEBUG_TSK  1
#define DEBUG_CAN  0
#define DEBUG_WIFI 1

extern SemaphoreHandle_t xSerialMutex;

//#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t drv_cpu = 0;
//#else
  static const BaseType_t app_cpu = 1;
//#endif
// TODO: Assign WiFi to another core 

QueueHandle_t xQueue;

TaskHandle_t hAppTsk1  = NULL;
TaskHandle_t hAppTsk2  = NULL;
TaskHandle_t hAppTsk3  = NULL;
TaskHandle_t hPrintTsk = NULL;

// Dispatching Mailbox, can also be checked by tasks
void vPrintTsk( void *pvParameters ) 
{
    Data_t xMessage;    

    // Identify message source and place message on specific task's qeueue/struct
    while(1) 
    {     
      if (xQueueReceive(xQueue, &xMessage, portMAX_DELAY) == pdPASS) 
      {      
#if DEBUG_TSK          
          logf("\n %lu: %s: %lu", esp_log_timestamp(), xMessage.msg, (unsigned long)xMessage.value);
#endif          
          // Task dispatch 
          if (xMessage.sender == Task1) {}
          if (xMessage.sender == Task2) {}
          if (xMessage.sender == Task3) {}        
          if (xMessage.sender == xUART) {}
          if (xMessage.sender == xUDP)  {} // Update remote dashboard and itself
          if (xMessage.sender == xCAN)  {} // Read data from CAN bus and UDP to remote dashboard.             
      } 
    }
}


/****************************************************************
 *  Description: 
 * 
 *  Input: Gets RS232 data from Mail box
 * 
 *  Output: writes RS232 msg
 * 
 ****************************************************************
 */ 
void vTaskRS232( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = Task1;

  while(1) 
  {
    // Write application process here:
    RS232tx("Hello world \n");
    RS232rx();

    // Send received data to the message box if needed
    xMessage.value = 2001;    
    snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "TaskRS232");
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/****************************************************************
 *  Description: 
 * 
 *  Input: Poll UDP queue
 * 
 *  Output: write UDP msg
 * 
 ****************************************************************
 */ 
void vTaskUDP( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = Task2;

  while(1) 
  {
    // Write application process here
    readUDP();
    writeUDP(WindowsIP, WindowsPort, replyBuffer);
    
    // Send received data to the message box if needed
    xMessage.value = 2002;
    snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "TaskUDP");
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/****************************************************************
 *  Description: 
 * 
 *  Input: Poll CAN queue
 * 
 *  Output: Write CAN msg
 * 
 ****************************************************************
 */ 
void vTaskCAN( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = Task3;

  // CAN data to be send out 
  uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xBB, 0xAA, 0xBB, 0xAA};

  CanMessage canRxMsg;
  canRxMsg.canId = 0x7E8;       // CAN ID filter

  CanMessage canTxMsg;
  canTxMsg.canId = 0x18FF008B;
  canTxMsg.dlc = CAN_FRAME_MAX_DLC;
  memcpy(canTxMsg.payload, data, sizeof(data));
   

  while(1) 
  {
    // Write application process here
    if (readCAN(&canRxMsg))
    {       
      logf("\n\n< Reading CAN ID: 0x%x --> 0x", canRxMsg.canId);
      
      for (int i = 0; i < canRxMsg.dlc; i++) {
          logf("%X", canRxMsg.payload[i]);
      }      

      logf("\n");
      
      // Extract SPN by masking desired postion      
#if DEBUG_CAN      
      logf("\n* Motor Temp: %3dC Deg \r\n", canRxMsg.payload[SPN_MOTOR_TEMP]);  
#endif          
    }

    writeCAN(&canTxMsg);
    
    // Send received data to the message box if needed
    xMessage.value = canRxMsg.payload[SPN_MOTOR_TEMP];
    snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "TaskCAN");
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void setup() 
{
  Serial.begin(460800);
  Serial.setDebugOutput(true); 
  delay(1000);  
  xSerialMutex = xSemaphoreCreateMutex();

  xQueue = xQueueCreate(50, sizeof(Data_t)); 
  if (xQueue != NULL) 
  {   // higher priority may starve loop
      xTaskCreatePinnedToCore(vPrintTsk, "PrintTask", 4096, NULL, 2, &hPrintTsk, drv_cpu); 
  }

  initFwRevision();
  mountFS();
  initI2C();
  initUARTx();
  initCAN();
  initSPI();
  initEth();
  
  // Connect to WiFi networks
  initWifiSTA();     // Should timeout and be before initWifiAP
  //initWifiAP();
  initWebServer();


  // TASK: add task handlers to place tasks on block during file upload and fw update.
  xTaskCreatePinnedToCore(vTaskRS232, "AppTsk1", 4096, NULL, 3, &hAppTsk1, app_cpu);
  xTaskCreatePinnedToCore(vTaskUDP, "AppTsk2", 4096, NULL, 3, &hAppTsk2, app_cpu);
  // Higher priority tasks here: CAN
  xTaskCreatePinnedToCore(vTaskCAN, "AppTsk3", 4096, NULL, 4, &hAppTsk3, drv_cpu);

  // Time sensitive task
  xTaskCreate(hwTaskLED,"LEDTask", 2048, NULL, 1, NULL);

  logf("Setup Completed. \n");

}


// RTOS priority 1 manage background processes.
void loop() 
{
  mEthernet();
  mWebServer();
  //mOTAreset();


#if DEBUG_LOOP 
  logf("\n # %lu: \n", millis());
#endif
}


