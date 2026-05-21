#include "drivers.h"
#include "includes.h"
#include <HardwareSerial.h>

#define DEBUG_UART 0

extern SemaphoreHandle_t xSerialMutex;

// Serial RS-232
static HardwareSerial SerialRS232(1);


// UART 0
void initUARTx()
{
  // Set UART for RS-232 interface
  SerialRS232.begin(RS232_BAUD, SERIAL_8N1,RS232_RX_PIN,RS232_TX_PIN);  
  SerialRS232.onReceive(RS232rx); 
}


// Interrupt based data must use Message box
int32_t RS232rx()
{
  Data_t xMessage;
  char data = 0;

  // Drain entire UART1 buffer to prevent FIFO overflow
  while (SerialRS232.available() > 0)
  {
    data = SerialRS232.read();  // Read all available bytes
  }

  // Interrupt based HW can send rx data to the message box

#if USE_DRVR_MBOX 
  xMessage.sender = xUART;
  xMessage.value = data;
  snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "RS232");
  xQueueSend(xQueue, &xMessage, (TickType_t)500);
#endif

  return data;
}

// Send UART data to an RS-232 interface, 
// same as printf() but can be configured independently
void RS232tx(const char* msg)
{
  // Process RS-232 data   
  if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
      SerialRS232.print(msg);
      xSemaphoreGive(xSerialMutex);
  }
}

