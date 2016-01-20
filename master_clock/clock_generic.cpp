/*
    Master Clock - Drives an IBM Impulse Secondary clock movement
    using the International Business Machine Time Protocols,
    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
    By Phil Hord,  This code is in the public domain Sept 9, 2013
*/

//_____________________________________________________________________
//                                                             INCLUDES

/* System configuration */
#define INCLUDE_DSTINFO
#include "setup.h"

#include "Arduino.h"
#include <stdarg.h>
#include <stdint.h>
#include "console.h"
#include "clock_generic.h"
#include "ntp.h"
#include "Udp.h"

//_____________________________________________________________________
//                                                           LOCAL VARS

static int uj, h, m, s ;       ///< Current date/time
byte a = LOW , b = LOW ;       ///< Desired A and B signal levels
byte d = LOW , e = LOW ;       ///< Desired D and E signal levels
byte f = LOW;                  ///< Current F signal level

static int aForce = 0 ;        ///< Force A pulse by operator control
static int bForce = 0 ;        ///< Force B pulse by operator control
static int dForce = 0 ;        ///< Force D pulse by operator control
static int eForce = 0 ;        ///< Force E pulse by operator control
static int realTick = 0;       ///< The number of ticks since we started

// Clock holds: Positive values suspend pulses, decrements once per second
static long int holdCountAB = 0;    ///< AB pulses
static long int holdCountDE = 0;    ///< DE pulses

// static int fallTime = 4;       // 400ms fall time default
// static int riseTime = 6;       // 600ms rist time default
#define fallTime 4
#define riseTime 6

static bool isDST = false;          ///< Currently in DST
static int doDST = 0;               ///< DST transition pending today
static unsigned long firstnow = 0 ; ///< First NTP result

typedef enum State {
	reset ,                // Reset ticker to 0 to sync with exact second
	rise ,                 // Rising edge of pulse
	riseWait ,             // Wait for pulse-rise time
	fall ,                 // Falling edge of pulse
	fallWait ,             // Wait for pulse-fall time
} State ;

State state = rise ;           ///< Protocol chain state tracker.


//_____________________________________________________________________
// Increment the tick counter.  This is called once every 100ms by the
// hardware interrupt or main executive function.
void ticker() {
  ++realTick;
}

//_____________________________________________________________________
// Tick accessor.  Because the 'tick' variable is modified during a
// hardware interrupt, it is not safe to read or write this variable
// outside of the interrupt routine unless we disable interrupts
// first.  These functions provide safe access to the tick variable.

//_____________________________________
// Read the current tick variable.
int getTick() {
  noInterrupts() ;
  int x = realTick ;
  interrupts();
  return x;
}

//_____________________________________
// Read number of elapsed ticks relative to timer variable
int elapsed( int timer ) {
  return getTick() - timer ;
}

//_____________________________________
// Return a timer tick from some point in the future
int getFuture( int nSeconds ) {
  return getTick() + nSeconds * 10 ;
}

//_____________________________________
// Test if a timer has expired (is in the past)
bool expired( int timer ) {
  return elapsed(timer) >= 0;
}

//_____________________________________
// Reset the tick counter to 0
void syncTime() {
  state = reset ;
}

//_____________________________________________________________________
// Time accessors
// Let other functions get and set the clock time

int getHours()   { return h; }
int getMinutes() { return m; }
int getSeconds() { return s; }
bool getDST()    { return isDST; }

void setHours( int hours ) { if (h != hours) { h = hours ; if (0 != firstnow) setSavedTimeValue(h, m) ;  } }
void setMinutes( int minutes ) { if (m != minutes) { m = minutes ; if (0 != firstnow) setSavedTimeValue(h, m) ; } }
void setSeconds( int seconds ) { s = seconds ; }

void incHours( )   { if (++h > 23) h = 0 ; if (0 != firstnow) setSavedTimeValue(h, m) ; }
void incMinutes( ) { if (++m > 59) {m = 0 ; if (++h > 23) h = 0 ; } if (0 != firstnow) setSavedTimeValue(h, m) ; }
void incSeconds( ) { if (++s > 59) { s = 0 ; incMinutes() ; }  }

void decHours( )   { if (--h < 0) h = 23 ; if (0 != firstnow) setSavedTimeValue(h, m) ; }
void decMinutes( ) { if (--m < 0) {m = 59 ; if (--h < 0) h = 23 ; } if (0 != firstnow) setSavedTimeValue(h, m) ; }
void decSeconds( ) { if (--s < 0) { s = 59 ; decMinutes() ; } }

void clockHold(int s1, int s2)   { holdCountAB += s1; holdCountDE += s2; }

// Unix time starts on Jan 1 1970, not Jan 1 1900 In seconds, that's a 2208988800 difference:

#define seventyYears 2208988800UL

static void setTimeFromNtp( int64_t ntpnow ) {
  static unsigned long lastnow = 0 ;      ///< Previous NTP result
  static long totalDrift = 0;             ///< Total seconds lost or gained
  int toffset, st, ct;
  // Subtract seventy years to get UNIX time
  unsigned long now = (unsigned long)(ntpnow - seventyYears);

  // print Unix time:
  p("Unix time = %lu\r\n", now);

  // Offset to local time
  now += (TIME_OFFSET + (isDST ? DST_OFFSET : 0)) * 60L ;

  unsigned hh = (now % 86400L) / 3600 ;
  unsigned mm = (now  % 3600) / 60 ;
  unsigned ss = now % 60 ;
  uj = now / 86400L;

  p("\r\nLocal date/time via NTP = %d %02u:%02u:%02u\r\n", uj, hh , mm , ss );

  long int delta = (now % 3600L ) - (m*60+s) ;
  p("Adjusting time by %d seconds.\r\n" ,  delta ) ;

  //-- Accumulate error history
  if (0 != lastnow) {
    totalDrift += delta ;

    //-- Report error history
    p("Seconds since first NTP: %ld   Since previous NTP: %ld   Total delta: %ld\r\n",
      now - firstnow, now - lastnow, totalDrift ) ;
  }

  if (uj != lastnow / 86400L) {
    int j;
    // Day change, DST checks needed
    for (j = 0; j < DSTLENGTH - 1 && uj >= DSTINFO(j); j++) ;
    if ( --j >= 0 ) {
      p("DST table index/value %d %d\r\n", j, DSTINFO(j));
      if ( 0 == lastnow ) {
        // First NTP sets DST indicator
        int jj = j;
        if (uj == DSTINFO(j) && hh < DST_TRANSITION_TIME) jj++;
        isDST = (jj % 2) == 0;
        p("Initial DST status: %d\r\n", isDST);
        if (isDST) now += 60 * 60;
        hh = (now % 86400L) / 3600 ;
        mm = (now  % 3600) / 60 ;
      }
      if ( uj == DSTINFO(j) && hh < DST_TRANSITION_TIME ) {
        // Today is the day, set things up to do DST switch
        doDST = (j % 2) ? -1 : 1;
        p("DST transition at %d today to %d\r\n", DST_TRANSITION_TIME, doDST > 0);
      }
    }
    else
      p("Date preceeds info; DST operations disabled\r\n");
  }

  if ( getSavedTimeValue(&st) ) {
    ct = (hh * 60) + mm;
    toffset = ct - st;
    p("Saved time %d %02d:%02d delta %d\r\n", st, st / 60, st % 60, toffset);
  } else {
    p("Saved time invalid\r\n");
    st = -1;
  }

  // First known time sync?
  if ( 0 == firstnow ) {
    if (st >= 0) {
      // Compute how far we have to move hands to
      // make up for time we were out of service
      if (toffset >= 12 * 60) toffset -= 12 * 60;
      else if (toffset <= -12 * 60) toffset += 12 * 60;
      if (toffset < -2 * 60) toffset = 12 * 60 + toffset;
      if (toffset < -2) {
        holdCountAB += toffset * 60;
        holdCountDE += toffset * 60;
      }
      else if (toffset > 2) {
          aForce += toffset;
          bForce += toffset;
          dForce += toffset;
          holdCountAB += 5;
          holdCountDE += 5;
      }
    }
    firstnow = now;
  }

  lastnow = now;

  setSeconds(ss) ;
  setMinutes(mm) ;
  setHours(hh) ;
}

//_____________________________________________________________________
// Signal accessors
// Let callers force A and B pulses

void sendPulsesA(int c) { aForce += c; }
void sendPulsesB(int c) { bForce += c; }
void sendPulsesD(int c) { dForce += c; }
void sendPulsesE(int c) { eForce += c; }

bool getA() { return a ; }        // Read the last A-signal level
bool getB() { return b ; }        // Read the last B-signal level
bool getD() { return d ; }        // Read the last D-signal level
bool getE() { return e ; }        // Read the last E-signal level
bool getF() { return f ; }        // Read the current F-signal level

//_____________________________________________________________________
//                                                        TIME PROTOCOL
//
// These functions actually implement the time protocol.

//_____________________________________
// Called once per second.  Advances second and minute counters.
// Decrements clock hold values. Performs actual DST change at 2:00AM.
// Compute current value for signal F.
static void markTime()
{
    incSeconds() ;
    if (holdCountAB > 0) holdCountAB-- ;
    if (holdCountDE > 0) holdCountDE-- ;
    if ( doDST != 0 && h >= DST_TRANSITION_TIME) {
      if (isDST != (doDST > 0)) {
#if DST_OFFSET >= 0
        if ( isDST ) {
          holdCountAB += DST_OFFSET * 60;
          holdCountDE += DST_OFFSET * 60;
        }
        else {
          aForce += DST_OFFSET;
          bForce += DST_OFFSET;
          dForce += DST_OFFSET;
        }
#else
       if ( isDST ) {
          aForce -= DST_OFFSET;
          bForce -= DST_OFFSET;
          dForce -= DST_OFFSET;
        }
        else {
          holdCountAB -= DST_OFFSET * 60;
          holdCountDE -= DST_OFFSET * 60;
        }
#endif
        // f will correct itself at 5:00AM
       }
      isDST = doDST > 0;
      doDST = 0;
    }
    int newF = (57 == m && s >= 57) ||
               (58 == m && (s <= 3 || (s <= 9 && (5 == h || 17 == h)))) ? HIGH : LOW;
    if (newF != f) {
      setState(newF);
      f = newF;
    }
}

//_____________________________________
// Implement the E-signal protocol.
// The 'E' signal is used to get to minute 59 instead of D.
// This also takes longer and blocks out other pulses
int checkE() {
    if ( eForce ) { eForce-- ; return HIGH ; }
    if (holdCountDE > 0) return LOW ;
    if ( 59 == m && 0 == s) return HIGH;
    return LOW ;
}

//_____________________________________
// Implement the A-signal protocol.
// Returns HIGH or LOW depending on what the A-signal output should
// based on the current time.
//
// The 'A' signal is raised once per minute at zero-seconds, and on
// every odd second between 10 and 50 during the 59th minute.
//
// Note: If 'aForce' is non-zero, return HIGH.
int checkA() {
    if (holdCountAB > 0) return LOW ;

    // Manual run based on serial input
    if ( aForce ) { aForce-- ; return HIGH ; }

    // Pulse once per minute
    if ( s == 0 ) return HIGH ;

    if ( m == 59 ) {
      if ( s >= 10 && s <= 50 ) {
        // On even seconds from 10 to 50, pulse A
        if ( ( s & 1 ) == 0 ) return HIGH ;
      }
    }
    return LOW ;
}

//_____________________________________
// Implement the B-signal protocol.
// Returns HIGH or LOW depending on what the B-signal output should
// based on the current time.
//
// The 'B' signal is raised once per minute at zero-seconds for each
// minute except for minutes 50 to 59.
//
// Note: If 'bForce' is non-zero, return HIGH.
int checkB() {
    if (holdCountAB > 0) return LOW ;

    // Manual run based on serial input
    if ( bForce ) { bForce-- ; return HIGH ; }

    // pulse once per minute from 00 to 49
    if ( m > 49 ) return LOW ;

    if ( s == 0 ) return HIGH ;
    return LOW ;
}

//_____________________________________
// Implement the D-signal protocol.
// Returns HIGH or LOW depending on what the D-signal output should
// based on the current time.
//
// The 'D' signal is raised once per minute at zero-seconds,
// except if the 'E' signal is active, in which case D is not
// raised. Note that the code requires that the value of e be
// determined before this routine is called, so always call
// checkE before checkD.
//
int checkD() {
    // Do nothing if 'E' is HIGH
    if ( HIGH == e ) return LOW;

    if (holdCountDE > 0) return LOW ;

    // Manual run based on serial input
    if ( dForce ) { dForce-- ; return HIGH ; }

 
    // pulse once per minute
    if ( s == 0 ) return HIGH ;

    return LOW ;
}

typedef enum Ntp_State {
    ntp_idle ,   	      ///< NTP is waiting to be triggered
    ntp_cron ,          ///< Auto-trigger NTP request
    ntp_request , 	    ///< Sending NTP request
    ntp_response ,      ///< Waiting for NTP response
    ntp_completed ,     ///< NTP completed; waiting for reset
    ntp_oneshot ,       ///< Try a short NTP request (manual intervention)
} Ntp_State ;

static Ntp_State ntpState = ntp_idle ;

void triggerNtp() {
	ntpState = ntp_oneshot ;
}

void checkNtp() {
    static int ntpResponseTimeout = 0 ;
    static int ntpRequestGuardTime = 0 ;
    static int retries ;

//	p("\r%d:  %d  %d  %d    ", ntpState , realTick, ntpResponseTimeout , retries ) ;
    switch ( ntpState ) {
    case ntp_idle :
        if ( m == 58 ) ntpState = ntp_cron ;
        break ;

    case ntp_cron:
	      ntpRequestGuardTime = getFuture( 15 * 60 ) ;    // Prevent multiple automatic requests within 15 minutes
        retries = 30 ;
        ntpState = ntp_request ;
        break ;

    case ntp_oneshot:
        retries = 4 ;
        ntpState = ntp_request ;
        break ;

    case ntp_request :
        sendNtpRequest() ;
	      ntpResponseTimeout = getFuture( 10 ) ;  	// timeout if we don't get a response in 10 seconds
	      ntpState = ntp_response ;
        break ;

    case ntp_response :
        if ( expired(ntpResponseTimeout) ) {
            ntpState = ntp_request ;
	          if ( --retries < 0 ) ntpState = ntp_completed ;
        }
	      else {
            int64_t ntpTime ;
	          if ( readNtpResponse( ntpTime ) ) {
                setTimeFromNtp(ntpTime);
	              ntpState = ntp_completed ;
	          }
	      }
        break ;

    case ntp_completed :
        if ( expired(ntpRequestGuardTime) ) {
	          ntpState = ntp_idle ;
        }
        break ;
    }
}

void clockSetup() {
	ntpSetup() ;
	triggerNtp() ;
}

//_____________________________________
// the service routine runs over and over again forever:
void service() {
  static int subTimer = 0;       ///< State counter per centisecond
  consoleService() ;
  checkNtp() ;

  switch (state) {
  default:
  case reset:
    subTimer = getTick() ;
  case rise:
    e = checkE() ; // Has to come first
    a = checkA() ;
    b = checkB() ;
    d = checkD() ;

    if (0 != firstnow) sendSignal( a , b, d, e ) ; // Send output pulses (if any)

    showTime() ; // Report time and signals to serial port

    state = riseWait ;
    break ;

  case riseWait:
    if ( elapsed(subTimer) < riseTime ) break ;
    subTimer += riseTime ;
    state = fall ;
    break ;

  case fall:
    if (0 != firstnow) sendSignal( LOW , LOW, LOW, LOW ) ; // End output pulses
    showSignalDrop() ;

    state = fallWait;
    break;

  case fallWait:
    if ( elapsed(subTimer) < fallTime ) break ;
    subTimer += fallTime ;

    markTime() ;                 // Advance time

    state = rise;
    break;
  }
}
