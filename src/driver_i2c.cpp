#include "drivers.h"
#include "includes.h"
#include <Wire.h>

extern SemaphoreHandle_t xSerialMutex;

// Use when a buffere is needed 
static unique_ptr<uint8_t[]> i2cBuffer(new uint8_t[64]);


void initI2C()
{
  // Set as Master
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);           // 100 kHz Speed
  memset(i2cBuffer.get(), 0, 64);  // Clear buffer
}


uint8_t I2CTx(unsigned long i2cData)
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
    
  //Wire.write(buf, sizeof(buf));
  Wire.write(buf[0]);
  Wire.write(buf[1]);
  Wire.write(buf[2]);
  Wire.write(buf[3]);

  // Signal End of Transmission
  uint8_t status = Wire.endTransmission(true);

  if (status == 0) {
     logf(" I2C Tx OK \n");
  } else {
     logf(" I2C Tx Error: %d \n", status);
  }
   
  return status;
}


uint8_t I2CRx() 
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
      //log_print_buf(temp, idx);
      logf("\n Bytes received from I2C device: %d, data: ", idx);

      for (idx = 0; idx < bytesReceived; idx++)
      {
          logHex(temp[idx]);
      }

      logf("\n");
  }

  return bytesReceived;
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


// Simplified Write just for testing
void testI2CWrite()
{
  Wire.beginTransmission(0x55);
  Wire.write(0x12);
  Wire.write(0x34);
  Wire.write(0x56);
  uint8_t status = Wire.endTransmission(true);

  logf("I2C write status: %d\n", status);
}