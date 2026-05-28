#include "drivers.h"
#include "includes.h"
#include <SPI.h>

extern SemaphoreHandle_t xSerialMutex;


void initSPI()
{
  // Manual pin reasignment for Xiao - clk, miso, mosi, ss
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  // Set Slave Select mode.
  pinMode(SPI.pinSS(),OUTPUT);

#if DEBUG_SPI
  logf("MOSI: ");
  logf("%d \n", MOSI);
  logf("MISO: ");
  logf("%d \n", MISO);
  logf("SCK: ");
  logf("%d \n", SCK);
  logf("SS: ");
  logf("%d \n", SS); 
  logf("SPI SS: GPIO");
  logf("%d \n", SPI.pinSS());
#endif    
}


void spiCommand(byte data) 
{
  //use it as you would the regular arduino SPI API
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI.pinSS(), LOW);   //pull SS slow to prep other end for transfer (commented out, hardwired)
  SPI.transfer(data);
  digitalWrite(SPI.pinSS(), HIGH);  //pull ss high to signify end of data transfer (commented out, hardwired)
  SPI.endTransaction();
}

