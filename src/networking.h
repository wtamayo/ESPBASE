#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include "includes.h"
#include "utils.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <SPI.h>

#define WIFI_UDP 1

// Access Point
static const char* host = "IoT-AP";
static const char* ssidAP = "Xiao";
static const char* passwordAP = "";

// Station: Client connects to my home wifi network
// WiFi UDP
static const char* hostname= "IoTStation";     // ESP32 Device Name
static const char* ssid = "VICMAR";            // Info about host network
static const char* password = "6043656101vm"; 
static IPAddress gateway(192, 168, 0, 1);
static IPAddress subnet(255, 255, 255, 0);
static IPAddress dns1(192, 168, 0, 1);
static IPAddress dns2(8, 8, 8, 8);

#if WIFI_UDP
// For WiFi based UDP communication
static IPAddress localIP(192, 168, 0, 105);  // ESP32 IP for Wifi
//static WiFiUDP udpTx;
//static WiFiUDP udpRx;
static WiFiUDP udp;
#else
// For Ethernet based UDP communication
static IPAddress localIP(192, 168, 0, 105);  // ESP32 IP for Ethernet
static EthernetUDP udpTx;
static EthernetUDP udpRx;
#endif

// Ethernet UDP
static uint8_t mac[6];

// The IP address on the Windows PC/Server where the ESP32 
// will send messages to.
static IPAddress WindowsIP(192, 168, 0, 135); 

// The UDP port on the Windows PC/Server where the ESP32 
// will send messages to.
static uint16_t WindowsPort = 1202;

// The IP address of the device that last sent a UDP packet to the ESP32. 
// It is filled by remoteIP = udpRx.remoteIP();
static IPAddress remoteIP = IPAddress();     

// The UDP source port of the device that last sent a UDP packet to the ESP32. 
// It is filled by remotePort = udpRx.remotePort();.
static uint16_t remotePort;

// The ESP32 UDP listening/source port.
// udpRx.begin(localPort) and udpTx.begin(localPort)
static uint16_t localPort = 8888;            


// buffers for receiving and sending data
static char packetBuffer[512];  // buffer to hold incoming packet,
static char replyBuffer[512] = "ESP32-ACK";                 // a string to send back


void writeUDP(IPAddress remoteIP, uint16_t remotePort, const char* tBuffer); 
uint64_t readUDP();
void mEthernet();
void tUDP();

void initWifiAP();
boolean initWifiSTA();
void initEth();
void timoWifiSta_CallBack(void* arg);

extern QueueHandle_t xQueue;

// Wifi timed events
static esp_timer_handle_t hTimWifiSta;

// Configure timer
static esp_timer_create_args_t timWifiSta = {
    .callback = &timoWifiSta_CallBack,
    .arg = &hTimWifiSta,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "OneShotTimer"
};

typedef enum {
  disable,
  enable,
  connected,
  disconnected,
  error
} Status_t;

static struct netStatus {
  Status_t enWiFiSTA = disable;
  Status_t enWiFiAP  = disable;
  Status_t enEth     = disable;
} netsta;

#endif