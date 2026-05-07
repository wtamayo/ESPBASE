#ifndef _UTILS_H_
#define _UTILS_H_

#include "includes.h"
#include <LittleFS.h> 
#include <FS.h>

// Custom timers
void timerCreateStart(esp_timer_create_args_t timerEvent, esp_timer_handle_t* hTimer, Timeoutus_t tout);

// File System
void mountFS(); 
void readFileFS(const char* filename);
bool deleteFileFS(const char* filePath);
void suspendTasks();
void resumeTasks();

// Log/debug/print
void log(char* msg);
void logln(char* msg);
void logHex(uint32_t msg);
void logf(const char* fmt, ...);


#endif