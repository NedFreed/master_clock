// clock_generic.h
//
// Helpers for the Time Protocol
//  
//    Master Clock - Drives an IBM Impulse Secondary clock movement
//    using the International Business Machine Time Protocols,
//    Service Instructions No. 230, April 1, 1938,Form 231-8884-0
//    By Phil Hord,  This code is in the public domain Sept 9, 2013 
//    
// This header file defines the external interfaces to the Time
// Protocol code.  The implementations of these functions are
// different on the Arduiono and the PC.  That is, there is one
// implementation for the Arduino, and there is another
// implementation for the PC.

//________________________________________________________________
// Tick counter
//
// The ticker function is called at precise times by the main
// program somehow.  This is how we keep track of time.  On
// the Arduino this function is called by a hardware interrupt.
// On the PC it is called by a simple clock watcher.
//
void ticker() ;

void speedUp() ;
void slowDown() ;

//________________________________________________________________
// User I/O
//
// These functions are used to "print" messages on the console
// and to read user input from the console.  On the arduino,
// the console is the serial port.  On the PC, the console
// is the shell terminal (keyboard and screen).
//
void sendString( const char * str ) ;
char readKey();

//________________________________________________________________
// Raise/lower digital IO pins
//
// The time protocol uses two signal lines, A and B or D and E.
// These variables hold the desired state of these two line pairs.
// To raise a signal line, set the corresponding variable to 1.
// To lower the signal line, set the corresponding variable
// to 0.  Call the function sendSignal to actually raise or
// lower each signal line.

void sendSignal( int a, int b, int d, int e ) ;

// Digital IO for more stateful IBM synchronous clocks

void setState( int f );

//________________________________________________________________
// Run the clock loop
//
// We want to run all the time, but we do not "own" the whole
// process.  The main process calls this routine repeatedly
// as long as our program is running.  This is where we do
// all the "work" of the program.

void service() ;
void clockSetup();

//________________________________________________________________
// Time accessors
// Let other functions get and set the clock time

int getHours() ;     ///< Return the current 'hours' time on the clock
int getMinutes() ;   ///< Return the current 'minutes' time on the clock
int getSeconds() ;   ///< Return the current 'seconds' time on the clock
bool getDST() ;      ///< Return current DST status

void setHours( int hours ) ;     ///< Set the clock 'hours' value
void setMinutes( int minutes ) ; ///< Set the clock 'minutes' value
void setSeconds( int seconds ) ; ///< Set the clock 'seconds' value

void incHours( ) ;
void incMinutes( ) ;
void incSeconds( ) ;

void decHours( ) ;
void decMinutes( ) ;
void decSeconds( ) ;

void syncTime() ;    ///< Zero the tick counter to sync the clock tick

//_____________________________________________________________________
// Signal accessors
// Let callers force A and B pulses

void sendPulsesA(int c) ;
void sendPulsesB(int c) ;
void sendPulsesD(int c) ;
void sendPulsesE(int c) ;

void clockHold(int s1, int s2) ;

bool getA( ) ;        // Read the last A-signal level
bool getB( ) ;        // Read the last B-signal level
bool getD( ) ;        // Read the last D-signal level
bool getE( ) ;        // Read the last E-signal level
bool getF( ) ;        // Read the current F-signal level

int getSavedTimeValue(int *t);
void setSavedTimeValue(int h, int m);


