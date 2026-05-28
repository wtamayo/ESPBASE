
#ifndef _INCLUDES_H_
#define _INCLUDES_H_

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>

// MCU and 3CS versions
const float fwVersions[2] = {1.0,0};

// Microseconds timers
typedef uint64_t Timeoutus_t;

// RTOS utils
#define USE_TASK_MBOX   1
#define USE_DRVR_MBOX   1
#define USE_RTOS_TASK   1
#define printout_INFO   0

// Timeouts
#define WATCHDOG_TIMEOUT  5  // In seconds
#define WIFISTA_TIMEOUT   5

// Debug flags
#define DEBUG_TSK       1
#define DEBUG_CAN       0
#define DEBUG_GPIO      0
#define DEBUG_I2C       0
#define DEBUG_UART      0
#define DEBUG_SPI       0
#define DEBUG_UDP       0
#define DEBUG_ETH       0
#define DEBUG_WIFI      0
#define DEBUG_WEB_SRV   0

// Device writers
typedef enum {
  xI2C,
  xSPI,
  xUART,
  xUDP,
  xCAN,
  Task1,
  Task2,
  Task3
} DataSource_t;

// Data places on the mailbox by devices
typedef struct {
  int64_t value;
  char msg[50];
  DataSource_t sender;
} Data_t;

typedef enum {
  toWatchdog,
  toWifista
} TimedEvents_t;

// Timeouts for custom events in microseconds
const Timeoutus_t timOut1s = 1 * 1000000;
const Timeoutus_t timOut5s = 5 * 1000000;
const Timeoutus_t timOut10s = 10 * 1000000;
const Timeoutus_t timOut60s = 60 * 1000000;
const Timeoutus_t timOut300s = 300 * 1000000;

// CAN SPNs
#define SPN_MOTOR_TEMP  0
#define SPN_COLANT_TEMP 1

// CAN IDs
#define CAN_ID_FILTER 0x7E8


#endif
