#include "utils.h"
#include <cstdarg>
#include <stdio.h>


SemaphoreHandle_t xSerialMutex;

void timerCreateStart(esp_timer_create_args_t timerEvent, esp_timer_handle_t* hTimer, Timeoutus_t tout) 
{
  esp_timer_create(&timerEvent, hTimer);   // must delete when expires
  esp_timer_start_once(*hTimer, tout); 

  // In case you need periodic
  //esp_timer_start_periodic(esp_timer_handle_t timer, uint64_t period);  // repeats on expiring
}

void mountFS() 
{
  size_t totalBytes;
  size_t usedBytes;

    if(!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        if (!LittleFS.format()) {
            Serial.println("Failed to format LittleFS");
        } else {
           Serial.println("LittleFS formatted successfully");
           LittleFS.begin();
        }
    } 
    else 
    {
        totalBytes = LittleFS.totalBytes();
        usedBytes = LittleFS.usedBytes();
        Serial.println("LittleFS file system mounted");
        Serial.print("LFS total space: ");
        Serial.println(String(totalBytes) + " Bytes");
        Serial.print("LFS used space: ");
        Serial.println(String(usedBytes) + " Bytes");
        Serial.print("LFS available space: ");
        Serial.println(String(totalBytes - usedBytes)  + " Bytes");
    }     
}


// The file size may be larger than the data in it.
void readFileFS(const char* filename) 
{
   // inline read file
  File file = LittleFS.open(filename, "r");
  int size = file.size();
  size_t bytesRead = 0;

  logf("Reading flashed file: %c of size: %d", filename, size);

  if (!file) 
  {
      logf("could not open file for reading \n");
  } 
  else 
  {      
      while (file.available()) 
      {             
          //logf("%X ", file.read());         // Process file here CAN/UDP    
          logHex(file.read());
          bytesRead++;             
          // yield();
      }
      
      logf("Amount of data in file: %c \n", bytesRead);    
      file.close();
  }
}

// Attempt to delete the file
bool deleteFileFS(const char* filePath) 
{
  if (LittleFS.exists(filePath)) 
  {    
      if (LittleFS.remove(filePath)) 
      {
          logf("File deleted successfully \n");
          return true;
      } 
      else 
      {
          logf("Failed to delete the file. \n");
      }
  } 
  else 
  {
      logf("File does not exist. \n");
  }
  
  return false;
}


void log(char* msg) 
{
    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
      Serial.print(msg);
      xSemaphoreGive(xSerialMutex);
    }
}

void logln(char* msg) 
{
    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
      Serial.println(msg);
      xSemaphoreGive(xSerialMutex);
    }
}

void logHex(uint32_t msg) 
{
    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
      Serial.print(msg, HEX);
      xSemaphoreGive(xSerialMutex);
    }
}

void logf(const char* fmt, ...)
{
    char buffer[128];

    va_list args;
    va_start(args, fmt);                 // start reading args
    vsnprintf(buffer, sizeof(buffer), fmt, args);  // format string
    va_end(args);                        // cleanup

    Serial.print(buffer);
}