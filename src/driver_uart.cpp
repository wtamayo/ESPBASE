#include "drivers.h"
#include "includes.h"
#include <HardwareSerial.h>

extern SemaphoreHandle_t xSerialMutex;

// Serial RS-232
static HardwareSerial UART1(1);


// UART 0
void initUARTx()
{
  // Set UART for RS-232 interface
  UART1.begin(RS232_BAUD, SERIAL_8N1,RS232_RX_PIN,RS232_TX_PIN);  
  UART1.onReceive(UART1Rx); 
}


// Interrupt based data must use Message box
int32_t UART1Rx()
{
  Data_t xMessage;
  int data = 0;

  // Drain entire UART1 buffer to prevent FIFO overflow
  while (UART1.available() > 0)
  {
    // Read all available chars available
    data = UART1.read();  
    logf("RS232 Data received %d", data);
  }

  // Interrupt based HW can send rx data to the message box

#if USE_DRVR_MBOX 
  xMessage.sender = xUART;
  xMessage.value = data;
  snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "UART1");
  xQueueSend(xQueue, &xMessage, (TickType_t)500);
#endif

  return data;
}

// Send UART data to an RS-232 interface, 
// same as printf() but can be configured independently
void UART1Tx(const char* msg)
{
  // Process RS-232 data   
  if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
      UART1.print(msg);
      xSemaphoreGive(xSerialMutex);
  }
}

