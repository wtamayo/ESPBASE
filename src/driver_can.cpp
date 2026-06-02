#include "drivers.h"
#include "includes.h"
#include <ESP32-TWAI-CAN.hpp>

extern SemaphoreHandle_t xSerialMutex;

// Timeout waiting upto 100ms for CAN data
// This is a blocking wait, so add that to
// scheduler task profiling behaviour.
#define BLOCK_WAIT  100

void initCAN() 
{
  // Set pins
	ESP32Can.setPins(CAN_TX, CAN_RX);
	
  // It is also safe to use .begin() without .end() as it calls it internally
  if (ESP32Can.begin(ESP32Can.convertSpeed(CAN_SPEED), CAN_TX, CAN_RX, 10, 10)) 
  {
      logf("CAN bus started \n");
  } 
  else 
  {
      logf("CAN bus failed \n");
  }
}


//void writeCAN(uint32_t CANID, idfSize_t sizeId, uint8_t dataLength, uint64_t payload) 
bool writeCAN(CanMessage* message)
{	
  CanFrame txFrame = { 0 };
	txFrame.identifier = message->canId;                // 0x18FF147A => 47A 11bit ID
	txFrame.extd = message->idSize;                     // 0 = 11bit ID, 1 = 29bit ID
	txFrame.data_length_code = message->dlc;            // How many bytes in paylod, max is 8

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
      pByte = (char*)&message->payload + i;
      index = (txFrame.data_length_code-1)-i;
      txFrame.data[index] = *pByte;               
  }	

  bool txOk = ESP32Can.writeFrame(txFrame, 10);

#if DEBUG_CAN     
  // Single logf call to prevent corrupting UART buffer on context switch
  char canLog[96] = {0};
  char hexBuf[32] = {0};

  for (int i = 0; i < txFrame.data_length_code; i++) {
      snprintf(hexBuf + (i*3), sizeof(hexBuf) - (i*3), "%02X ", txFrame.data[i]);
  }

  snprintf(canLog, sizeof(canLog), "\n\n> Writting CAN ID: 0x%x --> %s%s\n",
           txFrame.identifier,
           hexBuf,
           txOk ? "" : "\n> CAN Tx Error");

  // single mutex lock, single flush, atomic output     
  logf("%s", canLog);  
#endif   

  return txOk;
}


// Read CAN message and filer the canId desired
bool readCAN(CanMessage* message)
{
    CanFrame rxFrame;

    if (!ESP32Can.readFrame(rxFrame, BLOCK_WAIT)) {
        return false;
    }

    if (rxFrame.identifier != message->canId) {
        return false;
    }

    message->dlc = rxFrame.data_length_code;

    for (uint8_t i = 0; i < message->dlc; i++) {
        message->payload[i] = rxFrame.data[i];
    }

    return true;
}


