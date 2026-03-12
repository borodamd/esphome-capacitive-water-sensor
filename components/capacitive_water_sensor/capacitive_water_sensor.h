/*
  CapacitiveSensor.h v.04 - Capacitive Sensing Library for 'duino / Wiring
  https://github.com/PaulStoffregen/CapacitiveSensor
  http://www.pjrc.com/teensy/td_libs_CapacitiveSensor.html
  http://playground.arduino.cc/Main/CapacitiveSensor
  Copyright (c) 2009 Paul Bagder  All right reserved.
  Version 2015-03-15 - Paul Stoffregen - added interface to allow any pins

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef CapacitiveSensor_h
#define CapacitiveSensor_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// -----------------------------------------------------------------------------
// ----- Pin mapping constants, beyond this point -----
#if defined(__AVR__)
#define IO_REG_TYPE uint8_t
#endif

#if defined(__PIC32MX__)
#define IO_REG_TYPE uint32_t
#endif

#if defined(ESP8266) || defined(ESP32)
#define IO_REG_TYPE uint32_t
#endif

// -----------------------------------------------------------------------------

class CapacitiveSensor
{
  public:
    // Constructor
    CapacitiveSensor(uint8_t sendPin, uint8_t receivePin);
    
    // Public methods
    long capacitiveSensor(uint8_t samples);
    long capacitiveSensorRaw(uint8_t samples);
    void reset_CS_AutoCal(void);
    void set_CS_AutocaL_Millis(unsigned long time);
    void set_CS_Timeout_Millis(unsigned long time);
    
  private:
    // Private methods
    int CycleThis(void);
    
    // Private variables
    unsigned long total;
    unsigned long total1;
    unsigned int reg;
    unsigned long leastTotal;
    unsigned long lastCal;
    unsigned long CS_Timeout_Millis;
    unsigned long CS_AutocaL_Millis;
    int timeout;
    int timeoutOccurred;
    unsigned int loopTimingFactor;
    int error;
    
    // Pin mapping
    IO_REG_TYPE sBit;
    volatile IO_REG_TYPE *sReg;
    volatile IO_REG_TYPE *sOut;
    IO_REG_TYPE rBit;
    volatile IO_REG_TYPE *rReg;
    volatile IO_REG_TYPE *rIn;
    volatile IO_REG_TYPE *rOut;
};

#endif
