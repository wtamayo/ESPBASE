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
static IPAddress localIP(192, 168, 0, 105);  // IP for Wifi
static WiFiUDP udpTx;
static WiFiUDP udpRx;
#else
// For Ethernet based UDP communication
static IPAddress localIP(192, 168, 0, 105);  // IP for Ethernet
static EthernetUDP udpTx;
static EthernetUDP udpRx;
#endif

// Ethernet UDP
static uint8_t mac[6];
static IPAddress WindowsIP(192, 168, 0, 135); 
static IPAddress remoteIP = IPAddress();     // IP of remote device
static uint16_t remotePort;
static uint16_t localPort = 8888;            // 55555 on RTK
static uint16_t WindowsPort = 9999;

// buffers for receiving and sending data
static char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
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