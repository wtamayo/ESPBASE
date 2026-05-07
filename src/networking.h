#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include "includes.h"
#include "utils.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ESPmDNS.h>
#include <SPI.h>


static const char* host = "AKBarge";
static const char* ssidAP = "AKBarge";
static const char* passwordAP = "";

static const char* hostname= "akbarge";
static const char* ssid = "VICMAR";
static const char* password = "6043656101vm";

// Ethernet UDP
static uint8_t mac[6];
static IPAddress WindowsIP(192, 168, 1, 100);
static IPAddress localIP(192, 168, 1, 101);  // Replace by user entry in portal
static IPAddress remoteIP = IPAddress();     // IP of remote device
static uint16_t remotePort;
static uint16_t localPort = 8888;            // 55555 on RTK
static uint16_t WindowsPort = 9999;

// buffers for receiving and sending data
static char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
static char replyBuffer[512] = "AKBarge ACK";                 // a string to send back

static EthernetUDP udpTx;
static EthernetUDP udpRx;

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