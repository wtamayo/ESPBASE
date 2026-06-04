#ifndef _DRIVERS_H_
#define _DRIVERS_H_

#include "includes.h"
#include "utils.h"


/******* Hardware Defines *******/ 
#define BeatLed LED_BUILTIN

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
#define D1_PIN      D1    // Available

// I2C 
// D4 and D5 Internally wired on Xiao
#define SDA_PIN     D4
#define SCL_PIN     D5

// Xiao SPI PIN remapping SCK, MISO, MOSI, SS
#define SCK_PIN     D8    // GPIO_NUM_7  
#define MISO_PIN    D9    // GPIO_NUM_8  
#define MOSI_PIN    D10   // GPIO_NUM_9  

// SS_PIN could be Hardwired to VSS or VCC on SPI Chip.
#define SS_PIN      D0   

// ADC Sensor
#define ADC_PIN  A1

#endif

// UART
#define RS232_BAUD          9600
// CAN
#define CAN_SPEED           500
#define CAN_FRAME_MAX_DLC   8

static const int spiClk = 1000000; 

// I2C Device(s)
#define I2C_SLAVE_ADDR 0x55

extern QueueHandle_t xQueue;

// Define 11bit or 29 bit Identifier
 typedef enum {
    id11bit,
    id29bit
} idfSize_t;

struct CanMessage {
    uint32_t canId;
    uint8_t payload[8];
    uint8_t dlc;
    idfSize_t idSize = id29bit;
};

void initFwRevision();
void hwTaskLED(void *pvParameters);
// Serial 
void initUARTx();
void UART1Tx(const char* msg);
int32_t UART1Rx();
// I2C
void initI2C();
void scanI2C();
void mI2C();
// CAN
bool writeCAN(CanMessage* message);
bool readCAN(CanMessage* message);
//uint64_t readCAN(uint32_t PGN);
void initCAN();
// Test function
void tCAN();
// SPI
void initSPI();
// ADC
void initADC();
uint16_t readADC();
uint16_t readADCavg(uint8_t ADC_PIN_NUM);

// TODO: Move MAX31865 to its own file, this is low level
/*

// MAX31865 constants
#define MAX31865_RREF      430.0f   // PT100 board usually 430 ohm
#define MAX31865_RNOMINAL  100.0f   // PT100 = 100 ohm at 0°C

// For PT1000 use:
 #define MAX31865_RREF      4300.0f
// #define MAX31865_RNOMINAL  1000.0f

// MAX31865
void initMAX31865();
float readMAX31865TempC();
uint8_t readMAX31865Fault();
void clearMAX31865Fault();
*/

// Interface to access 25LC512 EEPROM chip. 
uint8_t eeprom25LC512ReadStatus();
bool eeprom25LC512WaitReady(uint32_t timeoutMs);

bool eeprom25LC512Read(uint16_t address, uint8_t *buffer, size_t length);
uint8_t eeprom25LC512ReadByte(uint16_t address);

bool eeprom25LC512Write(uint16_t address, const uint8_t *buffer, size_t length);
bool eeprom25LC512WriteByte(uint16_t address, uint8_t value);

bool eeprom25LC512Clear(uint8_t fillValue);

#endif