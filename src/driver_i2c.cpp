#include "drivers.h"
#include "includes.h"
#include <Wire.h>

extern SemaphoreHandle_t xSerialMutex;

void initI2C()
{
  Wire.begin();
}

void mI2C() 
{
  Data_t xMessage;
  unsigned long i2cData;      // Data to send to slave
  static bool i2cTx = true;   //Write message to the slave

  i2cData = 123456789;
  
  if (i2cTx) 
  {
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.printf("%lu", i2cData);
    uint8_t error = Wire.endTransmission(true);
    logf("End I2C Tx: %c", error);
    i2cTx = !i2cTx; 
  } 
  else 
  {
    //Read 16 bytes from the slave
    uint8_t bytesReceived = Wire.requestFrom(I2C_DEV_ADDR, 32);
    logf("Request from I2C device: %d", bytesReceived);
    if ((bool)bytesReceived) 
    {   //If received more than zero bytes
        uint8_t temp[bytesReceived];
        Wire.readBytes(temp, bytesReceived);
        log_print_buf(temp, bytesReceived);
    }
    i2cTx = !i2cTx;
  }

#if USE_DRVR_MBOX 
  xMessage.sender = xI2C;
  xMessage.value = 1001;
  snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "I2C");
  xQueueSend(xQueue, &xMessage, (TickType_t)500);
#endif
}

