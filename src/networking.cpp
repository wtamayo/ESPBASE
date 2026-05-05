#include "networking.h"


/************* WIFI *************/ 

void initWifiAP()
{
  WiFi.softAP(ssidAP, passwordAP);  
  logf("WiFi Direct available at Access Point IP: %c", WiFi.softAPIP());

  if (!MDNS.begin(host)) 
  {
    logf("Error setting up MDNS responder! \n");
    while (1) delay(1000);
  }
  logf("mDNS responder started \n");  
}


boolean initWifiSTA() 
{
  boolean result = false;
  unsigned long tStart = millis();
  // Start WiFi interface
  WiFi.begin(ssid, password);                                                      
  logf("Trying to connet to WiFi with SSID: %c", ssid);
    
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
      logf(" Unable to connet to %c", ssid);
  } 
  else 
  {
      // Show IP address that the ESP32 has received from router  
      logf(" Connected to LAN with IP address: %c \n", WiFi.localIP());

      // Give device a hostname so webpage can be easier to access
      if (!MDNS.begin(hostname))
      {  
          logf("Error starting mDNS \n");
          result = true;
      } 
      else 
      {
          logf("Device available at %c.local / or LAN IP address: ", hostname, WiFi.localIP());
      }
  }
  return result;
}


// Time sensitive events
void timoWifiSta_CallBack(void* arg) 
{
  logf("WiFi Station Timmed out \n");
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
      logf("Ethernet device not found. Can't run without hardware. \n");
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
      logf("Ethernet Started on Gateway: %c, and IP: %c \n", Ethernet.gatewayIP(), Ethernet.localIP());
  } 
  else 
  {
      //Serial.println("Ethernet cable is not connected.");
      logf("Ethernet cable is not connected. \n");
      netsta.enEth = disconnected;
  }

  // start UDP
  udpRx.begin(localPort);
  udpTx.begin(remotePort);
  logf("UDP service on: %d", udpRx.localPort());

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
      logf("<<Received UDP msg size %d <--", packetSize);

      // Identify remote IP
      remoteIP = udpRx.remoteIP();
      for (int i=0; i < 4; i++) 
      {
           //debug(remoteIP[i]);
           log(remoteIP[i]);
           if (i < 3) {              
              logf(".");
           }             
      }

      // Identify remote port
      logf(":");
      remotePort = udpRx.remotePort();
      logf("%d \n", remotePort);

      // read the packet into packetBuffer
      udpRx.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);      
      logf(", Payload: %d", packetBuffer);

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
      //debug(">>" + localIP.toString() + ":" + localPort);
      //debug("-->UDP msg to: ");
      //debug(remoteIP.toString());
      //debug(":");
      //debug(remotePort);
      //debug(", Payload: ");
      //debugln(tBuffer);

      logf(">> %c:%d -->UDP msg to: %c:%d, Payload: %c \n", localIP.toString(), localPort, remoteIP.toString(), remotePort, tBuffer);
      
      if (!udpTx.beginPacket(remoteIP, remotePort)) 
      {
          logf(">>Remote IP/Port Error");
      } 

      udpTx.write(tBuffer);

      if (!udpTx.endPacket()) 
      {          
          logf(">>UDP Rx Error \n");
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
