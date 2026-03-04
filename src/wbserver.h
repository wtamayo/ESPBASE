#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

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

//void update(Request &req, Response &res);
void initWebServer();

void mWebServer();
void mOTAreset();


#endif
