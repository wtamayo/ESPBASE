#include "drivers.h"
#include "includes.h"
#include <Wire.h>

extern SemaphoreHandle_t xSerialMutex;


void initI2C()
{
  // Set as Master
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);         // 100 kHz Speed
}

void mI2C() 
{
  Data_t xMessage;
  unsigned long i2cData;      // Data to send to slave
  static bool i2cTx = true;   //Write message to the slave

  i2cData = 0xDEADBEEF;
  
  if (i2cTx) 
  {
    logf("\n Transmitting I2C data: 0x%X \n", i2cData);
    static uint8_t i = 0;
    // Send Slave address
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    
    // Send as 4-byte binary (big-endian)
    uint8_t buf[4];
    buf[0] = (i2cData >> 24) & 0xFF;
    buf[1] = (i2cData >> 16) & 0xFF;
    buf[2] = (i2cData >> 8) & 0xFF;
    buf[3] = (i2cData) & 0xFF;
    
    // Dynamic data
    buf[3] += i++;

    Wire.write(buf, sizeof(buf));
    
    // Signal End of Transmission
    uint8_t status = Wire.endTransmission(true);

    if (status == 0) {
      logf("\n I2C Tx OK \n");
    } else {
      logf("\n I2C Tx Error: %d \n", status);
    }

    // Toggle so we alternate between TX and RX for testing
    i2cTx = !i2cTx;
  } 
  else 
  {
    // Request up to 32 bytes from the slave
    const uint8_t maxBuf = 32U;
    uint8_t bytesReceived = Wire.requestFrom((uint8_t)I2C_SLAVE_ADDR, maxBuf);

    if (bytesReceived) 
    {
        static uint8_t temp[maxBuf];
        uint8_t idx = 0;
        while (Wire.available() && idx < bytesReceived) {
            temp[idx++] = Wire.read();
        }
        log_print_buf(temp, idx);
        logf("\n Received from I2C device: %d, data: %d \n ", idx, idx);
    }
    i2cTx = !i2cTx;
  }

}


// Test only for development
void scanI2C()
{
  logf("\nScanning I2C bus...\n");

  for (uint8_t addr = 1; addr < 127; addr++)
  {
    Wire.beginTransmission(addr);
    uint8_t status = Wire.endTransmission();

    if (status == 0)
    {
      logf("I2C device found at 0x%02X\n", addr);
    }
  }

  logf("Scan complete.\n");
}