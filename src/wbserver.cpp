#include "wbserver.h"

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


void hServerLogin() 
{
  server.sendHeader("Connection", "close");  
  // inline read file
  File file = LittleFS.open(fLogin, "r");
  if (!file) {
    debugln("could not open file for reading");
  } else {
    while (file.available()) {      
      server.send(200, "text/html", file.readString());         
    }
    file.close();
  }
}


void hServerIndex() 
{
  server.sendHeader("Connection", "close");
  // inline read file
  File file = LittleFS.open(fIndex, "r");
  if (!file) {
    debugln("could not open file for reading");
  } else {
    while (file.available()) {      
      server.send(200, "text/html", file.readString());         
    }
    file.close();
  }
}

void hServerUpdate() 
{
  server.sendHeader("Connection", "close");
   // inline read file
  File file = LittleFS.open(fUpdate, "r");
  if (!file) {
    debugln("could not open file for reading");
  } else {
    while (file.available()) {      
      server.send(200, "text/html", file.readString());         
    }
    file.close();
  }
}

void hServerUpdateEnd() 
{
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  ESP.restart();
}

void hServerUpdatedStart() 
{
  HTTPUpload& upload = server.upload();
  Serial.println("Updating..");
 
  if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/      
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
      }
  } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
  }
}


/**************** User file Upload ****************/

void hServerUpload() 
{
  server.sendHeader("Connection", "close");
   // inline read file
  File file = LittleFS.open(fUpload, "r");
  if (!file) {
    debugln("could not open file for reading");
  } else {
    while (file.available()) {      
      server.send(200, "text/html", file.readString());         
    }
    file.close();
  }
}


// upload a file in to a flash file that is a multiple of 1436 bytes buffer 
// TODO: Block CAN and UDP tasks and other application tasks during file upload and OTA
void hServerUploadStart() 
{
  HTTPUpload& upload = server.upload();
  static size_t fStreamSize = 0;
  static size_t fStreamTotal = 0;

  // delete old file and clear upload buffer
  if (newFile) {
      deleteFileFS(fSave); 
      fStreamSize = 0;   
      newFile = false;      
  }

  File file = LittleFS.open(fSave, "a");
  
  if (!file) {
      debugln("could not open file for writting");
      return;
  }
  
  if (upload.status == UPLOAD_FILE_START) {
      debugln("Uploading: " + String(upload.filename));
  } else if (upload.status == UPLOAD_FILE_WRITE) {
      // The actual usefull data in the file will be upload.totalSize
      int bufferSize = sizeof(upload.buf);
      fStreamTotal += bufferSize;

      // Dont try to write beyond available file size.
      if (fStreamTotal >= MAX_FILE_SIZE) {
          debugln("Upload file size " + String(fStreamSize) + " larger than flash space " + String(MAX_FILE_SIZE));          
          debugln("Deleting truncated file:" + String(upload.filename));
          newFile = true;
          fStreamSize = 0; 
          fStreamTotal = 0;
          upload.status = UPLOAD_FILE_ABORTED;
          file.close();
          deleteFileFS(fSave);          
      } else {
          // May have to sanitize the last buffer upload.buf
          fStreamSize += file.write((const uint8_t*)upload.buf, bufferSize);
          memset(upload.buf, '\0', sizeof(upload.buf));          
      }

      debugln("Upload buffer written to file, file size: " + String(fStreamSize));

  } else if (upload.status == UPLOAD_FILE_END) {
      debugln("Upload completed: " + String(fStreamSize) + " size");
      newFile = true;
      fStreamSize = 0;   
      fStreamTotal = 0;          
      file.close();      
  }
}


void hServerUploadEnd() 
{
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  // Verify the file was written
  readFileFS(fSave);
}


void initWebServer()
{
  server.on("/", HTTP_GET, hServerLogin);
  server.on("/index", HTTP_GET, hServerIndex);  // There is no /index until there is a portal 
  
  // OTA Flash Update
  server.on("/update", HTTP_GET, hServerUpdate);
  server.on("/flash", HTTP_POST, hServerUpdateEnd, hServerUpdatedStart);

  // Upload a file and save to file-system
  server.on("/upload", HTTP_GET, hServerUpload);
  server.on("/save", HTTP_POST, hServerUploadEnd, hServerUploadStart);
  server.begin();
}

void mOTAreset() 
{
  if (shouldRestart) 
  {
      delay(1000);
      ESP.restart();
  }
}

void mWebServer() 
{
  server.handleClient();
}
