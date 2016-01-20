
/*
    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013

    Support added for modern UDP/NTP libraries, Simplex Electric clocks,
    IBM synchronous clocks, control via telnet, daylight savings time,
    tracking time losses due to power outages, and lots of minor changes.
    Ned Freed Sept 24, 2015
 */

/* System configuration */
#include "setup.h"

/* Library imports */
#include <EEPROM.h>
#include <TimerOne.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Dhcp.h>
#include <EthernetUdp.h>

/* Shared c++ code */
#include "clock_generic.h"

void sendSignal( int a, int b, int d, int e )
{
#ifdef OutputA
  digitalWrite(OutputA, a);   // Send A pulses
#endif
#ifdef OutputB
  digitalWrite(OutputB, b);   // Send B pulses
#endif
#ifdef OutputD
  digitalWrite(OutputD, d);   // Send D pulses
#endif
#ifdef OutputE
  digitalWrite(OutputE, e);   // Send E pulses
#endif
}

void setState( int f )
{
#ifdef OutputF
  digitalWrite(OutputF, f);   // Set F state
#endif
}

void sendString(const char *str )
{
  Serial.print( str ) ;
}

char readKey()
{
  if (Serial.available() == 0)
    return -1 ;
  return (char)Serial.read();
}

unsigned long us_per_tick = 100000 ;
void speedUp() {
  us_per_tick /= 2 ;
  Timer1.setPeriod(us_per_tick);    // Adjust tick speed
}

void slowDown() {
  us_per_tick *= 2 ;
  Timer1.setPeriod(us_per_tick);    // Adjust tick speed
}

// EEPROM write handling for hour storage
static int eeindex = 0;       ///< Location to write next
#define EEPROM_FIRSTLOC  0
#define EEPROM_LASTLOC  512

int getSavedTimeValue(int *ti)
{
  int j, e;
  long int t, tt;
  unsigned char good = true;

  t = 0;
  for (j = EEPROM_FIRSTLOC; j <= EEPROM_LASTLOC; j++) {
    if ((e = EEPROM[j]) > 127) {
      good = false;
      break;
    } else
      t += e;
  }
  if (good) {
    if (t > (EEPROM_LASTLOC - EEPROM_FIRSTLOC + 1) * 127L / 4) {
      // Sum is getting too large; reduce it                                    
      tt = t - t % (60*24);
      // Only allow one full reduction loop in case of EEPROM/code
      // issues - we don't want this loop to hang.
      j = 2;
      while (tt > 0 && j > 0) {
        if ((e = EEPROM[eeindex]) > tt) {
          EEPROM[eeindex] = e - tt;
          tt = 0;
        }
        else {
          EEPROM[eeindex] = 0;
          tt -= e;
        }
        if (++eeindex > EEPROM_LASTLOC) {
          eeindex = EEPROM_FIRSTLOC;
          j--;
        }
      }
      if (j == 0) {
        good = false;
        // First time initialization
        for (j = EEPROM_FIRSTLOC; j <= EEPROM_LASTLOC; j++)
          EEPROM[j] = 0;
        t = 0;
      }
    }
    *ti = (int)(t % (60*24));
  } else {
    for (j = EEPROM_FIRSTLOC; j <= EEPROM_LASTLOC; j++)
      EEPROM[j] = 0;
    *ti = 0;
  }
  return good;
}

void setSavedTimeValue(int h, int m)
{
  int c, j, t = h*60 + m;

  getSavedTimeValue(&c);
  t -= c;
  if (t >= -2 && t <= 2)
    return;
  if (t < 0) t += 60*24;
  j = 2;
  do {
    t += EEPROM[eeindex];
    if (t > 127) {
      EEPROM[eeindex] = 127;
      t -= 127;
    } else {
      EEPROM[eeindex] = t;
      t = 0;
    }
    if (++eeindex > EEPROM_LASTLOC) {
      eeindex = EEPROM_FIRSTLOC;
      j--;
    }
  } while (t > 0 && j > 0);
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  // initialize the digital pin as an output.
#ifdef OutputA
  pinMode(OutputA, OUTPUT);
#endif
#ifdef OutputB
  pinMode(OutputB, OUTPUT);
#endif
#ifdef OutputD
  pinMode(OutputD, OUTPUT);
#endif
#ifdef OutputE
  pinMode(OutputE, OUTPUT);
#endif
#ifdef OutputF
  pinMode(OutputF, OUTPUT);
#endif
  sendSignal( LOW , LOW, LOW, LOW ) ;
  Timer1.initialize(TIMER_PERIOD);   // initialize timer1 to 100ms period
  Timer1.attachInterrupt(ticker);    // attach timer overflow interrupt
  clockSetup() ;
}

// the loop routine runs over and over again forever:
void loop() {
  Ethernet.maintain();
  service();
}
