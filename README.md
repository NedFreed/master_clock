master_clock
============

Master clock protocol driver for IBM and Simplex slave clocks from an Arduino.
This is a fork of Phil Hord's original code, available at:

   https://github.com/phord/master_clock

This version has been extensively modified and in some places rewritten.

- The code now compiles with recent versions (1.6.5) of the Arduino IDE.
  Current UDP/NTP libraries are now used.

- Configuration settings have been moved to a new file, config.h. It should
  no longer be necessary to edit various constants scattered throughout the
  source.

- Support for Simplex clocks has been added. This support drives an additional
  pair of pins.

- Support for IBM synchronous clocks with second hands has been added. This
  support drives one additional pin.
    
- Daylight savings time support has been added. The tables and associated
  settings are in config.h.
  
- Support for tracking clock settings through power outages has been added.

- Support for talking to the clock via telnet has been added. The port used
  for this is specified in config.h. This presents the same console interface
  as the serial port.
  
- The console mode has been extensively revised. All commands can now be
  preceded with an unsigned number N. The available commands are:
  
  * N, n - manual trigger of NTP sync
  * <    - Slow the clock down for simulation runs
  * >    - Speeds the clock up for simulation runs
  * +    - Advance the digital clock minute
  * -, _ - Decrement the digital clock minute
  * H, h - Set hour value to N
  * M, m - Set minute value to N
  * S, s - Set second value to N
  * A, a - Send N A pulses to IBM slave clocks (default 1)
  * B, b - Send N B pulses to IBM slave clocks (default 1)
  * C, c - Send N combined A/B pulses  to IBM slave clocks (default 1)
  * D, d - Send N pulses to Simplex clocks (default 1)
  * E, e - Send reset command to Simplex clocks
  * f    - Turn on reset solenoid on IBM synchronous clocks 
  * F    - Turn off reset solenoid on IBM synchronous clocks
  * I, i - Hold IBM slave and Simply clocks for N seconds (default 3600)
  * U, u - Hold IBM slave clocks for N seconds (default 3600)
  * V, v - Hold Simplex clocks for N seconds (default 3600)

Important note: The PC test/simulator code is still present in the repo but
has NOT been maintained. As a result it is guaranteed not to build.
