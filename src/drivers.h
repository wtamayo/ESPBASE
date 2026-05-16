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

#ifdef DEVKITC
// UART0 default pins on ESP32-S3
// Serial UART0
#define RX_PIN      GPIO_NUM_44   // U0RXD
#define TX_PIN      GPIO_NUM_43   // U0TXD
// CAN/SPI Pins
// ESP32-S3 TWAI / CAN controller pins connected to external CAN transceiver
#define CAN_TX      GPIO_NUM_4    // TWAI TX -> CAN transceiver TXD
#define CAN_RX      GPIO_NUM_3    // TWAI RX <- CAN transceiver RXD
// Custom SPI pin mapping for external SPI device
#define SCK_PIN     GPIO_NUM_7
#define MISO_PIN    GPIO_NUM_8
#define MOSI_PIN    GPIO_NUM_9
#define SS_PIN      GPIO_NUM_46
#else
// Pinout for Xiao board
// UART0 Serial hardwired to USB-C Printf(). 
// UART1 pins (RS-232)
#define RS232_TX_PIN  D6  // GPIO_NUM_44
#define RS232_RX_PIN  D7  // GPIO_NUM_43 
// CAN/SPI Pins
#define CAN_TX		D3    // GPIO_NUM_4
#define CAN_RX		D2    // GPIO_NUM_3
// ADC/GPIO
//#define D0_PIN      D0    // Available (was SS_PIN on SW manged PIN)
//#define D1_PIN      D1    // Available
//I2C
//#define SDA_PIN     D4    // Internally wired SDA
//#define SCL_PIN     D5    // Internally wired SCL
// Xiao SPI PIN remapping SCK, MISO, MOSI, SS
#define SCK_PIN     D8    // GPIO_NUM_7  
#define MISO_PIN    D9    // GPIO_NUM_8  
#define MOSI_PIN    D10   // GPIO_NUM_9  
// SS_PIN Hardwired to VCC on SPI Chip.
#define SS_PIN      GPIO_NUM_46  // Using pin not exposed in Xiao
#endif

#define RS232_BAUD 9600
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