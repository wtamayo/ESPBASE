#ifndef _UTILS_H_
#define _UTILS_H_

#include "includes.h"

/************* File Locations *************/
#define MAX_FILE_SIZE  1048576           // 1MB = 1048576

static const char *fLogin  = "/server/login.html";
static const char *fIndex = "/server/index.html";
static const char *fUpdate = "/server/update.html";
static const char *fUpload = "/server/upload.html";
static const char *fSave = "/user/ufile.hex";

// Custom timers
void timerCreateStart(esp_timer_create_args_t timerEvent, esp_timer_handle_t* hTimer, Timeoutus_t tout);

// File System
void mountFS(); 
void readFileFS(const char* filename);
bool deleteFileFS(const char* filePath);
void suspendTasks();
void resumeTasks();

#endif