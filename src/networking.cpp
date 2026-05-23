#include "networking.h"

#define DEBUG_UDP   0
#define DEBUG_SPI   0
#define DEBUG_ETH   0
#define DEBUG_WIFI  0


/************* WIFI *************/ 

void initWifiAP()
{
  WiFi.mode(WIFI_AP);   // test
  WiFi.softAP(ssidAP, passwordAP);  
  logf("\n WiFi Direct available at Access Point IP: %s", WiFi.softAPIP().toString().c_str());

  if (!MDNS.begin(host)) 
  {
    logf("\n Error setting up MDNS responder! \n");
    while (1) delay(1000);
  }
  logf("\n mDNS responder started \n");  
}


boolean initWifiSTA() 
{
  boolean result = false;
  unsigned long tStart = millis();

  WiFi.mode(WIFI_STA);

  // Start WiFi interface
  if (!WiFi.config(localIP, gateway, subnet, dns1, dns2)) {
      logf("\n WiFi static IP config failed \n\n");
  }
  WiFi.begin(ssid, password);                                                      
  logf("\n Trying to connet to WiFi with SSID: %s \n\n", ssid);
    
  // Wait until WiFi is connected, but quit in 10 sec if no LAN found
  //timerCreateStart(timWifiSta, &hTimWifiSta, timOut10s);
  while (WiFi.status() != WL_CONNECTED && ((millis() - tStart) <= 10000)) 
  {   
        delay(200);    
        logf(".");
  } 

  if (WiFi.status() != WL_CONNECTED) 
  {
      //Serial.println(" Unable to connet to " + String(ssid));        
      logf("\n Unable to connet to %s \n\n", ssid);
  } 
  else 
  {
      // Show IP address that the ESP32 has received from router  
      logf("\n Connected to LAN with IP address: %s \n\n", WiFi.localIP().toString().c_str());

      // Give device a hostname so webpage can be easier to access
        if (!MDNS.begin(hostname))
        {  
          logf("\n Error starting mDNS \n");
          result = true;
        } 
        else 
        {
          logf("\n Device available at %s.local / or LAN IP address: %s \n\n", hostname, WiFi.localIP().toString().c_str());
        }

#if WIFI_UDP
      udpRx.begin(localPort);
      udpTx.begin(localPort);

      logf("\n WiFi UDP service on: %d \n", localPort);
#endif

  }
  return result;
}


// Time sensitive events
void timoWifiSta_CallBack(void* arg) 
{
  logf("\n WiFi Station Timmed out \n");
  esp_timer_delete(*(esp_timer_handle_t *)arg); 
}


void initEth()
{
  esp_efuse_mac_get_default(mac);
  Ethernet.init(SPI.pinSS());
  Ethernet.begin(mac, localIP);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) 
  {
      logf("\n Ethernet device not found. Can't run without hardware. \n");
      netsta.enEth = error;
      return;
  } 
  else 
  {
      netsta.enEth = enable;
  }
  
  if (Ethernet.linkStatus() == LinkON) 
  {
      netsta.enEth = connected;
      logf("\n Ethernet Started on Gateway: %s, and IP: %s \n", Ethernet.gatewayIP().toString().c_str(), Ethernet.localIP().toString().c_str());
  } 
  else 
  {
      //Serial.println("Ethernet cable is not connected.");
      logf("\n Ethernet cable is not connected. \n");
      netsta.enEth = disconnected;
  }

  // start UDP
#if !WIFI_UDP
  udpRx.begin(localPort);
  udpTx.begin(remotePort);
  logf("UDP service on: %d", localPort);
#endif

  // Give device a hostname so webpage can be easier to access
  // Not requiered - commented out.
  /*
  if (!MDNS.begin(hostname)) {                          
      Serial.println("Error starting mDNS \n");
  } else {
      Serial.println( "Access " + String(hostname) + ".local/ or the IP address into a browser to access portal. \n");
  }
  */
}


// Check the UDP Rx buffer for any data. 
uint64_t readUDP()
{
  uint64_t data = 0;

#if WIFI_UDP  
    if (WiFi.status() != WL_CONNECTED) {
        logf("\n WiFi not connected, UDP not sent \n");
        return 0;
    }     
#else         
    if (netsta.enEth != connected) {
        logf("\n Ethernet not avaialbe");          
        return 0;
    } 
#endif  

  // if there's data available, read a packet
  int packetSize = udpRx.parsePacket();       
      
  if (packetSize > 0) 
  {
      logf("\n << Received UDP msg size %d <--", packetSize);

      // Identify remote IP
      remoteIP = udpRx.remoteIP();

      for (int i=0; i < 4; i++) 
      {           
           logf("%d", remoteIP[i]);
           if (i < 3) {              
              logf(".");
           }             
      }

      // Identify remote port      
      remotePort = udpRx.remotePort();
      logf(":%d \n", remotePort);

      // Clear buffer before reading
      memset(packetBuffer, 0, sizeof(packetBuffer));

      // read the packet into packetBuffer
      int len = udpRx.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);      

      // Terminate buffer
      if (len > 0) {
          packetBuffer[len] = '\0';
      }

      logf(", Payload: %s", packetBuffer);

#if USE_DRVR_TASK
       // Send received data to the message box
      Data_t xDevMessage;
      xDevMessage.sender = xUDP;
      xDevMessage.value = 2222;
      xQueueSend(xQueue, &xDevMessage, (TickType_t)0);
#endif       
  }
  return data;
}

// Transmit buffer to remote udpTx 
void writeUDP(IPAddress remoteIP, uint16_t remotePort, const char* tBuffer) 
{
#if WIFI_UDP  
      if (WiFi.status() != WL_CONNECTED) {
          logf("\n WiFi not connected, UDP not sent \n");
          return;
      }
      logf("\n >> %s:%d --> UDP msg to: %s:%d, Payload: %s \n", WiFi.localIP().toString().c_str(), 
           localPort, remoteIP.toString().c_str(), remotePort, tBuffer);
#else         
      if (netsta.enEth != connected) {
          logf("\n Ethernet not avaialbe");          
          return;
      } 
      logf("\n >> %s:%d -->UDP msg to: %s:%d, Payload: %s \n", localIP.toString().c_str(), 
           localPort, remoteIP.toString().c_str(), remotePort, tBuffer);
#endif    

      if (!udpTx.beginPacket(remoteIP, remotePort)) 
      {
          logf(">>Remote IP/Port Error");
          return;
      } 

      size_t tLen = strlen(tBuffer);
      udpTx.write(reinterpret_cast<const uint8_t*>(tBuffer), tLen);

      if (!udpTx.endPacket()) 
      {          
          logf("\n >>UDP Rx Error \n");
      }
}
 
void rstCheckEth()
{
/*
    if(restartEthernet){
        restartEthernetFlag = false;
        esp_efuse_mac_get_default(macAddress);
        Ethernet.begin(macAddress, myIp);
        delay(200);
        udpRx.stop();
        udpRx.begin(myRxPort);
        resetWatchdogTimer();
    }
*/    
}
  
void mEthernet() 
{
  rstCheckEth();
}

// Test functio for UDP
void tUDP() 
{
  writeUDP(WindowsIP, WindowsPort, replyBuffer);
  delay(100);
  readUDP();
}
