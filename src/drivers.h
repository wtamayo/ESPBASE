#ifndef _DRIVERS_H_
#define _DRIVERS_H_

#include <Wire.h>
#include <SPI.h>
#include <ESP32-TWAI-CAN.hpp>
#include <HardwareSerial.h>
#include "includes.h"
#include "utils.h"


/******* Hardware Defines *******/ 
#define BeatLed LED_BUILTIN

// Serial RS-232
static HardwareSerial SerialRS232(1);

// Select pin nomenclature based on build
// Serial
#define RX_PIN D7        // GPIO_NUM_44
#define TX_PIN D6        // GPIO_NUM_43

// CAN/SPI Pins
#define CAN_TX		D3   // GPIO_NUM_4
#define CAN_RX		D2   // GPIO_NUM_3

// Xiao SPI PIN remapping SCK, MISO, MOSI, SS
#define SCK_PIN     D8    // GPIO_NUM_7  
#define MISO_PIN    D9    // GPIO_NUM_8  
#define MOSI_PIN    D10   // GPIO_NUM_9  
#define SS_PIN      D0    // GPIO_NUM_1  


#define BAUD 115200
#define CAN_SPEED   500
static const int spiClk = 1000000; 

// I2C Device(s)
#define I2C_DEV_ADDR 0x55

extern QueueHandle_t xQueue;

// Define 11bit or 29 bit Identifier
 typedef enum {
    id11bit,
    id29bit
} idfSize_t;

void initFwRevision();
void hwTaskLED(void *pvParameters); 
void initUARTx();
void initSPI();
void RS232tx(const char* msg);
int32_t RS232rx();
void mI2C();
void writeCAN(uint32_t CANID, idfSize_t sizeId, uint8_t dataLength, uint64_t payload);
uint64_t readCAN();
void initCAN();

// Test function
void tCAN();

#endif