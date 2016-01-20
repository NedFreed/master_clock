#ifndef __SETUP

#define __SETUP

/* Arduino pin usage. Commenting out #define will prevent and
   pin from being associated with that output */

/* Outputs for IBM impulse secondaries */
#define OutputA  8
#define OutputB  9

/* Outputs for Standard Electric secondaries */

#define OutputD  7
#define OutputE  6

/* Output for IBM synchronous secondaries */

#define OutputF  5

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield

#define MAC_ADDRESS {0x90, 0xA2, 0xDA, 0x0D, 0x66, 0x2F}

// Address of time server. Some possibilities:
// {132, 163, 4, 101} // time-a.timefreq.bldrdoc.gov NTP server
// {132, 163, 4, 102} // time-b.timefreq.bldrdoc.gov NTP server
// {132, 163, 4, 103} // time-c.timefreq.bldrdoc.gov NTP server

#define TIME_SERVER {10, 59, 230, 40}

// Local port used for NTP requests

#define TIME_LOCAL_PORT 8888

// Port used to talk to master clock

#define SERVER_PORT 2323

// Local offset from GMT in minutes (US/Pacific)

#define TIME_OFFSET (-8*60)

// Timer period - nominally 100ms (100000), but the Arduino clock isn't very accurate

#define TIMER_PERIOD 100249

#ifdef INCLUDE_DSTINFO

#include <avr/pgmspace.h>

// DST transitions at 2AM

#define DST_TRANSITION_TIME 2

// DST offset in minutes

#define DST_OFFSET 60

// DST change dates, expressed as # days since Jan 1 1970

#define DSTLENGTH 46

// United States DST transition dates - store in flash to save RAM

const uint16_t dstinfo[DSTLENGTH] PROGMEM = {
  16502, // 2015/3/8
  16740, // 2015/11/1
  16873, // 2016/3/13
  17111, // 2016/11/6
  17237, // 2017/3/12
  17475, // 2017/11/5
  17601, // 2018/3/11
  17839, // 2018/11/4
  17965, // 2019/3/10
  18203, // 2019/11/3
  18329, // 2020/3/8
  18567, // 2020/11/1
  18700, // 2021/3/14
  18938, // 2021/11/7
  19064, // 2022/3/13
  19302, // 2022/11/6
  19428, // 2023/3/12
  19666, // 2023/11/5
  19792, // 2024/3/10
  20030, // 2024/11/3
  20156, // 2025/3/9
  20394, // 2025/11/2
  20520, // 2026/3/8
  20758, // 2026/11/1
  20891, // 2027/3/14
  21129, // 2027/11/7
  21255, // 2028/3/12
  21493, // 2028/11/5
  21619, // 2029/3/11
  21857, // 2029/11/4
  21983, // 2030/3/10
  22221, // 2030/11/3
  22347, // 2031/3/9
  22585, // 2031/11/2
  22718, // 2032/3/14
  22956, // 2032/11/7
  23082, // 2033/3/13
  23320, // 2033/11/6
  23446, // 2034/3/12
  23684, // 2034/11/5
  23810, // 2035/3/11
  24048, // 2035/11/4
  24174, // 2036/3/9
  24412, // 2036/11/2
  24538, // 2037/3/8
  24776  // 2037/11/1
};

#define DSTINFO(k) pgm_read_word_near(dstinfo + k)
#endif

#endif
