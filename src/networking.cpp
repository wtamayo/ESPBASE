#include "networking.h"

#define DEBUG 0

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

/************* WIFI *************/ 

void initWifiAP()
{
  WiFi.softAP(ssidAP, passwordAP);
  Serial.print("WiFi Direct available at Access Point IP: ");
  Serial.println(WiFi.softAPIP());

  if (!MDNS.begin(host)) 
  {
    Serial.println("Error setting up MDNS responder!");
    while (1) delay(1000);
  }
  Serial.println("mDNS responder started \n");  
}


boolean initWifiSTA() 
{
  boolean result = false;
  unsigned long tStart = millis();
  // Start WiFi interface
  WiFi.begin(ssid, password);                                                     
  Serial.println("Trying to connet to WiFi with SSID: " + String(ssid));    
    
  // Wait until WiFi is connected, but quit in 10 sec if no LAN found
  //timerCreateStart(timWifiSta, &hTimWifiSta, timOut10s);
  while (WiFi.status() != WL_CONNECTED && ((millis() - tStart) <= 10000)) 
  {   
        delay(200);
        Serial.print(".");
  } 

  if (WiFi.status() != WL_CONNECTED) 
  {
      Serial.println(" Unable to connet to " + String(ssid));        
  } 
  else 
  {
      // Show IP address that the ESP32 has received from router  
      Serial.print(" Connected to LAN with IP address: ");
      Serial.println(WiFi.localIP());

      // Give device a hostname so webpage can be easier to access
      if (!MDNS.begin(hostname))
      {  
          Serial.println("Error starting mDNS");
          result = true;
      } 
      else 
      {
          Serial.print( "Device available at " + String(hostname) + ".local / or LAN IP address: ");
          Serial.println(WiFi.localIP()); 
      }
  }
  return result;
}


// Time sensitive events
void timoWifiSta_CallBack(void* arg) 
{
  debug("WiFi Station Timmed out");
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
      Serial.println("Ethernet device not found. Can't run without hardware.");
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
      Serial.print("Ethernet Started on Gateway: ");  
      Serial.print(Ethernet.gatewayIP());
      Serial.print(" and IP: ");
      Serial.println(Ethernet.localIP());
  } 
  else 
  {
      Serial.println("Ethernet cable is not connected.");
      netsta.enEth = disconnected;
  }

  // start UDP
  udpRx.begin(localPort);
  udpTx.begin(remotePort);
  Serial.print("UDP service on:");  
  Serial.println(udpRx.localPort());

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
  uint64_t data;

  // if there's data available, read a packet
  int packetSize = udpRx.parsePacket();       
      
  if ((netsta.enEth == connected) && packetSize) 
  {
      debug("<<Received UDP msg size ");
      debug(packetSize);
      debug("<--");

      // Identify remote IP
      remoteIP = udpRx.remoteIP();
      for (int i=0; i < 4; i++) 
      {
           debug(remoteIP[i]);
           if (i < 3) debug(".");
      }

      // Identify remote port
      debug(":");
      remotePort = udpRx.remotePort();
      debug(remotePort);

      // read the packet into packetBuffer
      udpRx.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      debug(", Payload: ");
      debug(packetBuffer);

#if USE_HAL_MBOX
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
      // send a reply to the IP address and port that sent us the packet we received
      debug(">>" + localIP.toString() + ":" + localPort);
      debug("-->UDP msg to: ");
      debug(remoteIP.toString());
      debug(":");
      debug(remotePort);
      debug(", Payload: ");
      debugln(tBuffer);
      
      if (!udpTx.beginPacket(remoteIP, remotePort)) 
      {
          debugln(">>Remote IP/Port Error");
      } 

      udpTx.write(tBuffer);

      if (!udpTx.endPacket()) 
      {
          debugln(">>UDP Rx Error");
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
