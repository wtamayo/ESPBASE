#include "drivers.h"
#include "includes.h"

#define DEBUG 0

extern SemaphoreHandle_t xSerialMutex;

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

void initUARTx()
{
  // Set UART for RS-232 interface
  // Configure MySerial0 on pins TX=D6 and RX=D7 
  SerialRS232.begin(BAUD, SERIAL_8N1,RX_PIN,TX_PIN);
  SerialRS232.onReceive(RS232rx); 
}

void initFwRevision() 
{
  logf("\n");
  logf("*************** AK Barge ************** \n");  
  logf("        MCU Firmware Rev: %c \n", String(fwVersions[0]));
  logf("        3CS Firmware Rev: %c \n", String(fwVersions[1]));
  logf("*************************************** \n");
}


// Interrupt based data must use Message box
int32_t RS232rx()
{
  Data_t xMessage;
  int32_t data;
  char rxBuffer[32]; 
  
  if (SerialRS232.available() > 0)
  {
    SerialRS232.readBytes(rxBuffer, sizeof(rxBuffer));
    // data = rxBuffer[0];

    // Interrupt based HW can send rx data to the message box
  }

#if USE_DRVR_MBOX 
  xMessage.sender = xUART;
  xMessage.value = 1000;
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


void initSPI()
{
  // Manual pin reasignment for Xiao - clk, miso, mosi, ss
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  // Set Slave Select mode.
  pinMode(SPI.pinSS(),OUTPUT);

#if DEBUG_SPI
  debug("MOSI: ");
  debugln(MOSI);
  debug("MISO: ");
  debugln(MISO);
  debug("SCK: ");
  debugln(SCK);
  debug("SS: ");
  debugln(SS); 
  debug("SPI SS: GPIO");
  debugln(SPI.pinSS());
#endif    
}


void spiCommand(byte data) 
{
  //use it as you would the regular arduino SPI API
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI.pinSS(), LOW);   //pull SS slow to prep other end for transfer
  SPI.transfer(data);
  digitalWrite(SPI.pinSS(), HIGH);  //pull ss high to signify end of data transfer
  SPI.endTransaction();
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


void writeCAN(uint32_t CANID, idfSize_t sizeId, uint8_t dataLength, uint64_t payload) 
{	
  CanFrame txFrame = { 0 };
	txFrame.identifier = CANID;                // 0x18FF147A => 47A 11bit ID
	txFrame.extd = sizeId;                     // 0 = 11bit ID, 1 = 29bit ID
	txFrame.data_length_code = dataLength;     // How many bytes in paylod, max is 8

  if (txFrame.data_length_code > TWAI_FRAME_MAX_DLC)
      txFrame.data_length_code = TWAI_FRAME_MAX_DLC;

  // Best to use 0xAA (0b10101010) instead of 0
	// to avoid bit-stuffing	
  memset(txFrame.data, 0xAA, sizeof(txFrame.data)); 
  
  char* pByte;
  uint8_t index;

  // MSB first
  for (int i = 0; i < txFrame.data_length_code; i++) 
  {
      pByte = (char*)&payload + i;
      index = (txFrame.data_length_code-1)-i;
      txFrame.data[index] = *pByte;               
  }	
	
  // Accepts both pointers and references 
  logf("\n >Writting CAN ID: 0x%x --> ", txFrame.identifier);
  
  for (int i=0; i<txFrame.data_length_code; i++) 
  {
       logHex(txFrame.data[i]);    
  }  


  // timeout defaults to 1 ms
  if (ESP32Can.writeFrame(txFrame, 10) == false) 
  {
      logf("\n >CAN Tx Error");
  }     
}


// No Message box needed
uint64_t readCAN() 
{
  Data_t xMessage;
  int64_t data = 0;
  CanFrame rxFrame;

  // You can set custom timeout, default is 1000
  if (ESP32Can.readFrame(rxFrame, 100)) 
  {
      //uint16_t PNG = (rxFrame.identifier & 0x00FFFF00) >> 8;
      logf("<Reading CAN ID: 0x%x <--", rxFrame.identifier);
        
      for (int i=0; i<rxFrame.data_length_code; i++) 
      {
           logHex(rxFrame.data[i]);
      }
      
      logf("\n");

      // Filter only desired CAN IDs and assign value to data
      //debug("Payload data: 0x%X \r\n", rxFrame.data);
      //if(rxFrame.identifier == 0x7E8) { 
      //   debug("Collant temp: %3d°C \r\n", rxFrame.data[3] - 40); 
      //}    
  }    

#if USE_DRVR_MBOX 
  xMessage.sender = xCAN;
  xMessage.value = 1003;
  snprintf(xMessage.msg, sizeof(xMessage.msg), "%s", "CANRx");
  xQueueSend(xQueue, &xMessage, (TickType_t)500);
#endif 

  return data;
}


void initCAN() 
{
  // Set pins
	ESP32Can.setPins(CAN_TX, CAN_RX);
	
  // It is also safe to use .begin() without .end() as it calls it internally
  if (ESP32Can.begin(ESP32Can.convertSpeed(CAN_SPEED), CAN_TX, CAN_RX, 10, 10)) 
  {
      Serial.println("CAN bus started");
  } 
  else 
  {
      Serial.println("CAN bus failed");
  }
}

// Test function not application function
void tCAN() 
{
  static uint32_t lastStamp = 0;
  uint32_t currentStamp = millis();    
  static bool write = false;

  // Set 29bit CANID: 18FF147A (max 0x1FFFFFFF) or 11bit CANID: 47A (max 7FF)
  uint32_t canid = 0x18FF008B; 
  uint64_t payload = 0xDEADBEEFBBAABBAA;

  if (currentStamp - lastStamp > 1000) 
  { 
      lastStamp = currentStamp;
      if (write) 
      {
          writeCAN(canid, id29bit, TWAI_FRAME_MAX_DLC, payload);             
      } 
      else 
      {
          readCAN();  // moved to a task
      }
      write = !write; 
  }
}
