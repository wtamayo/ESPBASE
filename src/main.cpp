/***************************************************************************************
 * 
 * Multi-task communication hub on ESP32:
 *  
 * Each interface runs in its own task:
 *  RS232 → vAppTsk1
 *  UDP → vAppTsk2
 *  CAN → vAppTsk3
 *  
 *  All tasks send messages to a central mailbox (queue)
 *  A dispatcher task (vPrintTsk) receives and processes/logs messages
 *  This is basically a producer → queue → consumer pattern
 * 
 * 
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

#define DEBUG 1

#if DEBUG
#define debug(x) Serial.print(x);
#define debugln(x) Serial.println(x);
#define debugx(x, base) Serial.print(x, base);
#else
#define debug(x)
#define debugln(x)
#define debugx(x, base)
#endif


//#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t drv_cpu = 0;
//#else
  static const BaseType_t app_cpu = 1;
//#endif
// TODO: Assign WiFi to another core 

#if USE_TASK_MBOX
QueueHandle_t xQueue;

TaskHandle_t hAppTsk1  = NULL;
TaskHandle_t hAppTsk2  = NULL;
TaskHandle_t hAppTsk3  = NULL;
TaskHandle_t hPrintTsk = NULL;

// Dispatching Mailbox, can also be checked by tasks
void vPrintTsk( void *pvParameters ) 
{
    Data_t xMessage;    
    char buffer[80];

    // Identify message source and place message on specific task's qeueue/struct
    while(1) 
    {     
      if (xQueueReceive(xQueue, &xMessage, (TickType_t)1000) == pdPASS) 
      {              
          snprintf(buffer, sizeof(buffer),"%lu: %s: %u", millis(), xMessage.msg, xMessage.value);
          Serial.println(buffer);

          // Task dispatch 
          if (xMessage.sender == Task1) {}
          if (xMessage.sender == Task2) {}
          if (xMessage.sender == Task3) {}        
          if (xMessage.sender == xUART) {}
          if (xMessage.sender == xUDP)  {} // Update remote dashboard and itself
          if (xMessage.sender == xCAN)  {} // Read data from CAN bus and UDP to remote dashboard.             
      } else {
          Serial.println("IPC Queue Failed!");
      }
    }
}
#endif

#if USE_RTOS_TASK

/****************************************************************
 *  Description: 
 * 
 *  Input: Gets RS232 data from Mail box
 * 
 *  Output: writes RS232 msg
 * 
 ****************************************************************
 */ 
void vAppTsk1( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = Task1;

  while(1) 
  {
    // Write application process here:
    RS232tx("Hello world");
    vTaskDelay(pdMS_TO_TICKS(500));
    RS232rx();

    // Send received data to the message box if needed
    xMessage.value = 2001;
    xMessage.msg = "AppTask1";
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(1000/ portTICK_RATE_MS);
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
void vAppTsk2( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = Task2;

  while(1) 
  {
    // Write application process here
    readUDP();
    vTaskDelay(pdMS_TO_TICKS(500));
    writeUDP(WindowsIP, WindowsPort, replyBuffer);

    // Send received data to the message box if needed
    xMessage.value = 2002;
    xMessage.msg = "AppTask2";
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(1000/ portTICK_RATE_MS);
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
void vAppTsk3( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = Task3;

  uint32_t canid = 0x18FF008B; 
  uint64_t payload = 0xDEADBEEFBBAABBAA;


  while(1) 
  {
    // Write application process here
    readCAN();
    vTaskDelay(pdMS_TO_TICKS(500));
    writeCAN(canid, id29bit, TWAI_FRAME_MAX_DLC, payload);
    
    // Send received data to the message box if needed
    xMessage.value = 2003;
    xMessage.msg = "AppTask3";
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(1000/ portTICK_RATE_MS);
  }
}

#endif

void setup() 
{
  Serial.begin(460800);
  delay(1000);
  Serial.println("Setup started.");

#if USE_TASK_MBOX
  xQueue = xQueueCreate(50, sizeof(Data_t)); 
  if (xQueue != NULL) 
  {   // higher priority may starve loop
    xTaskCreatePinnedToCore(vPrintTsk, "PrintTask", 2048, NULL, 2, &hPrintTsk, drv_cpu); 
  }
#endif

  initFwRevision();
  Wire.begin();
  mountFS();
  
  initUARTx();
  initCAN();
  initSPI();
  initEth();
  
  // Connect to WiFi networks
  initWifiSTA();     // Should timeout and be before initWifiAP
  initWifiAP();
  initWebServer();

#if USE_RTOS_TASK
    // TASK: add task handlers to place tasks on block during file upload and fw update.
    xTaskCreatePinnedToCore(vAppTsk1, "AppTsk1", 2048, NULL, 3, &hAppTsk1, app_cpu);
    xTaskCreatePinnedToCore(vAppTsk2, "AppTsk2", 2048, NULL, 3, &hAppTsk2, app_cpu);
    // Higher priority tasks here:
    xTaskCreatePinnedToCore(vAppTsk3, "AppTsk3", 2048, NULL, 4, &hAppTsk3, drv_cpu);
#endif

  // Time sensitive task
  xTaskCreate(hwTaskLED,"LEDTask", 2048, NULL, 1, NULL);

}

// RTOS priority 1 manage background processes.
void loop() 
{
  mEthernet();
  mWebServer();
  mOTAreset();
  
#if DEBUG_INFO
  char buffer[80];
  snprintf(buffer, sizeof(buffer),"\n # %lu: "\n", millis());
  Serial.println(buffer);
#endif
}

