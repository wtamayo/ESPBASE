#include "drivers.h"
#include "includes.h"
#include <SPI.h>

extern SemaphoreHandle_t xSerialMutex;
extern SemaphoreHandle_t xSPIMutex;

// 25LC512 EEPROM constants
// 512 Kbit = 64 KByte, 16-bit address, 128-byte page write size.
#define EEPROM_25LC512_SIZE_BYTES  65536UL
#define EEPROM_25LC512_PAGE_SIZE   128U

#define EEPROM_CMD_READ   0x03
#define EEPROM_CMD_WRITE  0x02
#define EEPROM_CMD_WREN   0x06
#define EEPROM_CMD_WRDI   0x04
#define EEPROM_CMD_RDSR   0x05
#define EEPROM_CMD_WRSR   0x01

#define EEPROM_STATUS_WIP 0x01  // Write In Progress bit
#define EEPROM_STATUS_WEL 0x02  // Write Enable Latch bit

#ifndef EEPROM_WRITE_TIMEOUT_MS
#define EEPROM_WRITE_TIMEOUT_MS 20U
#endif

static inline uint8_t eepromCsPin()
{
  return SPI.pinSS();
}

static inline void eepromSelect()
{
  if (xSemaphoreTake(xSPIMutex, portMAX_DELAY)) {
      digitalWrite(eepromCsPin(), LOW);
      xSemaphoreGive(xSPIMutex);
  }
}

static inline void eepromDeselect()
{
  if (xSemaphoreTake(xSPIMutex, portMAX_DELAY)) {
      digitalWrite(eepromCsPin(), HIGH);
      xSemaphoreGive(xSPIMutex);
  }
}

static void eepromWriteEnable()
{
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  eepromSelect();
  SPI.transfer(EEPROM_CMD_WREN);
  eepromDeselect();
  SPI.endTransaction();
}

static void eepromWriteDisable()
{
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  eepromSelect();
  SPI.transfer(EEPROM_CMD_WRDI);
  eepromDeselect();
  SPI.endTransaction();
}

uint8_t eeprom25LC512ReadStatus()
{
  uint8_t status;

  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  eepromSelect();
  SPI.transfer(EEPROM_CMD_RDSR);
  status = SPI.transfer(0x00);
  eepromDeselect();
  SPI.endTransaction();

  return status;
}

bool eeprom25LC512WaitReady(uint32_t timeoutMs)
{
  uint32_t start = millis();

  while (eeprom25LC512ReadStatus() & EEPROM_STATUS_WIP) {
    if ((millis() - start) >= timeoutMs) {
      return false;
    }
    delay(1);
  }

  return true;
}

bool eeprom25LC512Read(uint16_t address, uint8_t *buffer, size_t length)
{
  if (buffer == nullptr) {
    return false;
  }

  if ((uint32_t)address + length > EEPROM_25LC512_SIZE_BYTES) {
    return false;
  }

  if (!eeprom25LC512WaitReady(EEPROM_WRITE_TIMEOUT_MS)) {
    return false;
  }

  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  eepromSelect();
  SPI.transfer(EEPROM_CMD_READ);
  SPI.transfer((uint8_t)(address >> 8));
  SPI.transfer((uint8_t)(address & 0xFF));

  for (size_t i = 0; i < length; i++) {
    buffer[i] = SPI.transfer(0x00);
  }

  eepromDeselect();
  SPI.endTransaction();

  return true;
}

uint8_t eeprom25LC512ReadByte(uint16_t address)
{
  uint8_t value = 0xFF;
  eeprom25LC512Read(address, &value, 1);
  return value;
}

static bool eeprom25LC512WritePage(uint16_t address, const uint8_t *buffer, size_t length)
{
  if (buffer == nullptr || length == 0 || length > EEPROM_25LC512_PAGE_SIZE) {
    return false;
  }

  // Do not allow one page-write command to cross a 128-byte page boundary.
  if (((address & (EEPROM_25LC512_PAGE_SIZE - 1)) + length) > EEPROM_25LC512_PAGE_SIZE) {
    return false;
  }

  if (!eeprom25LC512WaitReady(EEPROM_WRITE_TIMEOUT_MS)) {
    return false;
  }

  eepromWriteEnable();

  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  eepromSelect();
  SPI.transfer(EEPROM_CMD_WRITE);
  SPI.transfer((uint8_t)(address >> 8));
  SPI.transfer((uint8_t)(address & 0xFF));

  for (size_t i = 0; i < length; i++) {
    SPI.transfer(buffer[i]);
  }

  eepromDeselect();
  SPI.endTransaction();

  return eeprom25LC512WaitReady(EEPROM_WRITE_TIMEOUT_MS);
}

bool eeprom25LC512Write(uint16_t address, const uint8_t *buffer, size_t length)
{
  if (buffer == nullptr) {
    return false;
  }

  if ((uint32_t)address + length > EEPROM_25LC512_SIZE_BYTES) {
    return false;
  }

  size_t written = 0;

  while (written < length) {
    uint16_t currentAddress = address + written;
    size_t pageOffset = currentAddress & (EEPROM_25LC512_PAGE_SIZE - 1);
    size_t bytesToPageEnd = EEPROM_25LC512_PAGE_SIZE - pageOffset;
    size_t bytesRemaining = length - written;
    size_t chunkLength = (bytesRemaining < bytesToPageEnd) ? bytesRemaining : bytesToPageEnd;

    if (!eeprom25LC512WritePage(currentAddress, buffer + written, chunkLength)) {
      return false;
    }

    written += chunkLength;
  }

  eepromWriteDisable();
  return true;
}

bool eeprom25LC512WriteByte(uint16_t address, uint8_t value)
{
  return eeprom25LC512Write(address, &value, 1);
}

bool eeprom25LC512Clear(uint8_t fillValue)
{
  uint8_t page[EEPROM_25LC512_PAGE_SIZE];
  memset(page, fillValue, sizeof(page));

  for (uint32_t address = 0; address < EEPROM_25LC512_SIZE_BYTES; address += EEPROM_25LC512_PAGE_SIZE) {
    if (!eeprom25LC512Write((uint16_t)address, page, sizeof(page))) {
      return false;
    }
  }

  return true;
}
