/***************************************************************************************
 * 
 * Multi-task communication hub on ESP32:
 *  
 * Each interface runs in its own task:
 *  RS232 → vTaskUART1
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

extern SemaphoreHandle_t xSerialMutex;

//#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t drv_cpu = 0;
//#else
  static const BaseType_t app_cpu = 1;
//#endif
// TODO: Assign WiFi to another core 

QueueHandle_t xQueue;

TaskHandle_t hTskRS232   = NULL;
TaskHandle_t hTskUDP     = NULL;
TaskHandle_t hTskCAN     = NULL;
TaskHandle_t hTskI2C     = NULL;
TaskHandle_t hTskSPI     = NULL;
TaskHandle_t hTskADC     = NULL;
TaskHandle_t hTskMsgQ    = NULL;


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
          if (xMessage.sender == xADC)  {}
          if (xMessage.sender == xSPI)  {}
          if (xMessage.sender == xI2C)  {}
          if (xMessage.sender == xUDP)  {} // Update remote dashboard and itself
          if (xMessage.sender == xCAN)  {} // Read data from CAN bus and UDP to remote dashboard.             
      } 
    }
}


/****************************************************************
 *  Description: ADC A1
 * 
 *  Input: Read ADC from pin A1
 * 
 *  Output: Averaged measured voltage
 * 
 ****************************************************************
 */ 
void vTaskADC( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = xADC;

  while(1) 
  {
    // Write application process here:
    int raw = readADCavg(ADC_PIN);

    // 4095 since is a 12 bit resolution ADC
    float mV = (raw * (3.3f / 4095.0f)) * 1000;

#if DEBUG_ADC
    logf("\n ADC Raw: %4d  Voltage: %.0fmV \n", raw, mV);
#endif    
    
    // Send received data to the message box if needed
    xMessage.value = mV;     
    snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "TaskADC");
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/****************************************************************
 *  Description: 
 * 
 *  Input: Gets SPI data from Mail box
 * 
 *  Output: Send and Receives I2C msg
 * 
 ****************************************************************
 */ 
void vTaskSPI( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = xSPI;

  while(1) 
  {
    // Write application process here:
    

    // Send received data to the message box if needed
    xMessage.value = 2006;    
    snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "TaskSPI");
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


/****************************************************************
 *  Description: 
 * 
 *  Input: Gets I2C data from Mail box
 * 
 *  Output: Send and Receives I2C msg
 * 
 ****************************************************************
 */ 
void vTaskI2C( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = xSPI;

  while(1) 
  {
#if DEBUG_I2C
    //Test I2C R/W when device present
    mI2C(); 
#endif    

    // Send received data to the message box if needed
    xMessage.value = 2005;    
    snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "TaskI2C");
    xQueueSend(xQueue, &xMessage, (TickType_t)500);
    vTaskDelay(pdMS_TO_TICKS(1000));
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
void vTaskUART1( void *pvParameters ) 
{
  Data_t xMessage;
  xMessage.sender = xUART;

  while(1) 
  {
    // Write application process here:
    UART1Tx("Hello world \n");
    UART1Rx();

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
  xMessage.sender = xUDP;

  while(1) 
  {
    // Write application process here
    if (isWiFiCon() || isEthernetCon()) {
        readUDP();
        writeUDP(WindowsIP, WindowsPort, replyBuffer);
    }
    
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
  xMessage.sender = xCAN;

  // CAN data to be send out 
  uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xBB, 0xAA, 0xBB, 0xAA};

  CanMessage canRxMsg;
  canRxMsg.canId = CAN_ID_FILTER;       // CAN ID filter

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
      xTaskCreatePinnedToCore(vPrintTsk, "PrintTask", 4096, NULL, 2, &hTskMsgQ, drv_cpu); 
  }

  initFwRevision();
  mountFS();
  initADC();
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
  xTaskCreatePinnedToCore(vTaskUART1, "TaskUART1", 4096, NULL, 3, &hTskRS232, app_cpu);
  xTaskCreatePinnedToCore(vTaskUDP, "TaskUDP", 4096, NULL, 3, &hTskUDP, app_cpu);
  // Higher priority tasks here: CAN
  xTaskCreatePinnedToCore(vTaskCAN, "TaskCAN", 4096, NULL, 4, &hTskCAN, drv_cpu);
  xTaskCreatePinnedToCore(vTaskI2C, "TaskI2C", 4096, NULL, 4, &hTskI2C, drv_cpu);
  xTaskCreatePinnedToCore(vTaskSPI, "TaskSPI", 4096, NULL, 4, &hTskSPI, drv_cpu);
  xTaskCreatePinnedToCore(vTaskADC, "TaskADC", 4096, NULL, 4, &hTskADC, drv_cpu);

  // Time sensitive task
  xTaskCreate(hwTaskLED,"LEDTask", 2048, NULL, 1, NULL);


  logf(" Setup Completed. \n");

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


