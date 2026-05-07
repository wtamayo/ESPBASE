#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include <WebServer.h>
#include <LittleFS.h> 
#include <Update.h>
#include <aWOT.h>
#include "includes.h"
#include "utils.h"

// Webserver
static uint16_t webServerPort = 80;

//static WiFiServer server(webServerPort);
static WebServer server(webServerPort);  
static Application webApp;

static char expectHeader[20] {};
static bool shouldRestart = false;

static bool newFile = true;

/************* File Locations *************/
#define MAX_FILE_SIZE  1048576           // 1MB = 1048576

static const char *fLogin  = "/server/login.html";
static const char *fIndex = "/server/index.html";
static const char *fUpdate = "/server/update.html";
static const char *fUpload = "/server/upload.html";
static const char *fSave = "/user/ufile.hex";

//void update(Request &req, Response &res);
void initWebServer();

void mWebServer();
void mOTAreset();


#endif
