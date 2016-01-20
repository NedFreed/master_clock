/*

 UDP interface for Arduino support

 Originally based on NtpUdp.c:
   created 4 Sep 2010
   by Michael Margolis
   modified 9 Apr 2012
   by Tom Igoe
   modified 8 Aug 2015 - modernize DHCP support
   by Ned Freed 
   This code is in the public domain.

 */

/* System configuration */
#include "setup.h"

#include <Arduino.h>
#include <IPAddress.h>
#include <SPI.h>
#include <Dhcp.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "console.h"

byte mac[] = MAC_ADDRESS;

IPAddress *timeServerAddress = NULL ;
unsigned int timeServerPort = 0 ;

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// A TCP server instance to accept commands over the network

EthernetServer Server = EthernetServer(SERVER_PORT);
EthernetClient Client;

void reportMac()
{
  p("MAC address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// Just a utility function to nicely format an IP address.
char *ip_to_str(IPAddress ipAddr)
{
  static char buf[16];

  sprintf(buf, "%d.%d.%d.%d", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

void udpSetup(unsigned char* addr , unsigned int port )
{
  reportMac();
  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    p("Failed to configure Ethernet with DHCP\r\n");
    // No point in carrying on, so do nothing forevermore:
    while(true);
  }
  p("IP address: %s\r\n", ip_to_str(Ethernet.localIP()));
  p("   Gateway: %s\r\n", ip_to_str(Ethernet.gatewayIP()));
  p("      Mask: %s\r\n", ip_to_str(Ethernet.subnetMask()));
  p("       DNS: %s\r\n", ip_to_str(Ethernet.dnsServerIP()));

  timeServerAddress = new IPAddress(addr[0], addr[1], addr[2], addr[3]);
  timeServerPort = port;
  p("NTP Server: %s:%d\r\n", ip_to_str(*timeServerAddress), timeServerPort);
  Udp.begin(TIME_LOCAL_PORT);
  Server.begin();
}

int sendUdp( char * data, int size )
{
  Udp.beginPacket(*timeServerAddress, timeServerPort);
  byte count = Udp.write((const unsigned char *)data,size);
  Udp.endPacket();
  return (int)count;
}

int readUdp( char * buf, int size )
{
  int count = Udp.parsePacket() ;
  if (count) Udp.read( buf, size ) ;
  return count ;
}

typedef enum Server_State {
    server_idle ,        ///< No connection
    server_connected     ///< Server connected
} Server_State ;

static Server_State serverState = server_idle ;

int readServer()
{
  switch (serverState) {
    case server_idle :
      if ((Client = Server.available()))
        serverState = server_connected ;
      else
        break;

    case server_connected :
      if (!Client.connected()) {
        Client.stop() ;
        serverState = server_idle ;
      }
      else if (Client.available())
        return Client.read();
      break;
  }
  return -1;
}

void writeServer(char *s)
{
  switch (serverState) {
    case server_idle :
      return;
    case server_connected :
      if (!Client.connected()) {
        Client.stop() ;
        serverState = server_idle ;
      }
      else Client.print(s) ;    
  }
}

