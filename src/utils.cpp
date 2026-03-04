#include "utils.h"

#define DEBUG 1

// Use debug whenever the message is not part of init
#if DEBUG
#define debug(x) Serial.print(x);
#define debugln(x) Serial.println(x);
#define debugx(x, base) Serial.print(x, base);
#else
#define debug(x)
#define debugln(x)
#define debugx(x, base)
#endif


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

  debugln("Reading flashed file:" + String(filename) + " of size:" + String(size));

  if (!file) 
  {
      debugln("could not open file for reading");
  } 
  else 
  {      
      while (file.available()) 
      {             
          //Serial.printf("%X ", file.read());         // Process file here CAN/UDP    
          debugx(file.read(), HEX);
          bytesRead++;             
          // yield();
      }
      debugln("Amount of data in file:" + String(bytesRead));    
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
          debugln("File deleted successfully");
          return true;
      } 
      else 
      {
          debugln("Failed to delete the file.");
      }
  } 
  else 
  {
      debugln("File does not exist.");
  }
  
  return false;
}
