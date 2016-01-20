/*
   console.cpp

   Implements the console user interface to the master clock.

    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013
*/

//_____________________________________________________________________
//                                                             INCLUDES
#include "Arduino.h"
#include <stdarg.h>
#include "clock_generic.h"
#include "console.h"
#include "Udp.h"

//_____________________________________________________________________
// Print formatted text to the console.
void p(const char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        sendString(tmp);
        writeServer(tmp);
}

// Print the current time and A/B signal levels on the console
void showTime() {
  unsigned int s = getSeconds();

  //-- Newline + whole time every minute
  if ( s == 0 )
    p("\r\n%02u%s:%02u:%02u ", getHours(), getDST() ? "D" : "", getMinutes(), s);
  else if ( (s % 10 ) == 0)
    p("%02u", s );
  else
    p("-");

  //-- Show the A/B/D/E pulse status, F level
  if (getA()) p("A");
  if (getB()) p("B");
  if (getD()) p("D");
  if (getE()) p("E");
  if (getF()) p("F");
}

// Report that a signal has dropped
void showSignalDrop() {
    p("%s", (getA()||getB()||getD()||getE())?"*":"");
}

void triggerNtp();
//_____________________________________
// Read user input and act on it
bool controlMode( char ch ) {
    static int arg = -1;
    int retval = false;
    if (isdigit( ch )) {
      if (arg < 0) arg = 0;
      arg = arg * 10 + (ch - '0');
      return false;
    }
    switch ( ch ) {
    // 'N' triggers the NTP protocol manually
    case 'N': case 'n': triggerNtp() ; retval = true ; break ;

    // Left-arrow (Less-Than) slows down the clock for simulation runs
    case '<': case ',': slowDown() ;   retval = true ; break ;

    // Right-arrow (Greater-Than) speeds up the clock for simulation runs
    case '>': case '.': speedUp() ;    retval = true ; break ;

    // PLUS advances the digital clock minute
    case '+': case '=': incMinutes() ; retval = true ; break ;

    // MINUS decrements the digital clock minute
    case '_': case '-': decMinutes() ; retval = true ; break ;

    // H, M, S set hours, minutes, seconds
    case 'H': case 'h': if (arg >= 0) setHours(arg);                retval = true ;      break ;
    case 'M': case 'm': if (arg >= 0) setMinutes(arg);              retval = true ;      break ;
    case 'S': case 's': if (arg >= 0) setSeconds(arg); syncTime() ; retval = true ;      break ;

    // 'Z' resets the digital clock seconds
    case 'z': case 'Z': setSeconds(0) ; syncTime() ; return true ;

    // A, B and C manually force the A/B output pulses but do not affect the internal clock
    // A and B add to the force count for the A and B signals.  C adds to both signals.
    case 'A': case 'a': if (arg < 0) arg = 1 ;    sendPulsesA(arg) ;                     break ;
    case 'B': case 'b': if (arg < 0) arg = 1 ;    sendPulsesB(arg) ;                     break ;
    case 'C': case 'c': if (arg < 0) arg = 1 ;    sendPulsesA(arg) ;  sendPulsesB(arg) ; break ;
    case 'D': case 'd': if (arg < 0) arg = 1 ;    sendPulsesD(arg) ;                     break ;
    case 'E': case 'e':                           sendPulsesE(1)   ;                     break ;

    case 'f':                                     setState(LOW);                         break ;
    case 'F':                                     setState(HIGH);                        break ;

    case 'I': case 'i': if (arg < 0) arg = 60*60; clockHold(arg, arg) ;                  break ;
    case 'U': case 'u': if (arg < 0) arg = 60*60; clockHold(arg, 0) ;                    break ;
    case 'V': case 'v': if (arg < 0) arg = 60*60; clockHold(0, arg) ;                    break ;
    default: break;
  }
  arg = -1;
  return retval ;
}

void consoleService() {
  char ch ;

  ch = readKey();
  if ( ch < 1 ) {
    ch = readServer();
    if (ch < 1) return;
  }

  if ( controlMode(ch) ) { p(" -> %02d:%02d:%02d ", getHours(), getMinutes(), getSeconds() );  }
}
