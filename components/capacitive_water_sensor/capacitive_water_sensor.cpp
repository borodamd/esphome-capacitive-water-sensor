/*
  CapacitiveSense.h v.04 - Capacitive Sensing Library for 'duino / Wiring
  https://github.com/PaulStoffregen/CapacitiveSensor
  http://www.pjrc.com/teensy/td_libs_CapacitiveSensor.html
  http://playground.arduino.cc/Main/CapacitiveSensor
  Copyright (c) 2009 Paul Bagder  All right reserved.
  Version 2015-03-15 - Paul Stoffregen - added interface to allow any pins
  Version 2016-02-26 - Paul Stoffregen - remove analogRead, only digital pins

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

#include "CapacitiveSensor.h"

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

CapacitiveSensor::CapacitiveSensor(uint8_t sendPin, uint8_t receivePin)
{
    // Initialize instance variables
    error = 1;
    loopTimingFactor = 310;
    CS_Timeout_Millis = (2000);
    CS_AutocaL_Millis = (20000);
    leastTotal = 0;
    lastCal = millis();
    
    // get pin mapping and port for send pin
    sBit = digitalPinToBitMask(sendPin);
    sReg = portOutputRegister(digitalPinToPort(sendPin));
    sOut = sReg;
    
    // get pin mapping and port for receive pin
    rBit = digitalPinToBitMask(receivePin);
    rReg = digitalPinToPort(receivePin);
    rIn = portInputRegister(rReg);
    rOut = portOutputRegister(rReg);
    rReg = portModeRegister(rReg);
    
    // reset pin modes
    *rReg &= ~rBit;  // Set receive pin to INPUT
    *sOut &= ~sBit;  // Set send pin LOW
    pinMode(sendPin, OUTPUT);  // Make send pin an output
    pinMode(receivePin, INPUT); // Make receive pin an input
}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

long CapacitiveSensor::capacitiveSensor(uint8_t samples)
{
    total = 0;
    if (samples == 0) return 0;
    for (uint8_t i = 0; i < samples; i++) {
        if (samples == 1) {
            total += capacitiveSensorRaw(1);
        } else {
            total += capacitiveSensorRaw(2);
        }
    }
    
    // Only calibrate if the calibration interval has passed
    if ( (millis() - lastCal > CS_AutocaL_Millis) && abs((long)(total - leastTotal)) < (long)(0.10f * (float)leastTotal) ) {
        leastTotal = total;
        lastCal = millis();
    }
    
    // Compensate for transmission delay
    total -= leastTotal;
    
    return total;
}

long CapacitiveSensor::capacitiveSensorRaw(uint8_t samples)
{
    total1 = 0;
    if (samples == 0) return 0;
    for (uint8_t i = 0; i < samples; i++) {
        if (CycleThis() < 1) {
            total1 += 0;
            timeoutOccurred = true;
        } else {
            total1 += (unsigned long)reg;
        }
    }
    
    if (timeoutOccurred) {
        if (total1 == 0) return -2;
        timeoutOccurred = false;
    }
    
    return total1;
}

void CapacitiveSensor::reset_CS_AutoCal()
{
    leastTotal = 9999999;
}

void CapacitiveSensor::set_CS_AutocaL_Millis(unsigned long time)
{
    CS_AutocaL_Millis = time;
}

void CapacitiveSensor::set_CS_Timeout_Millis(unsigned long time)
{
    CS_Timeout_Millis = time;
}

// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library

int CapacitiveSensor::CycleThis()
{
    // Disable interrupts
    noInterrupts();
    
    // Set receive pin to OUTPUT LOW
    *rReg |= rBit;          // Set receive pin to OUTPUT
    *rOut &= ~rBit;         // Set receive pin LOW
    delayMicroseconds(10);
    
    // Set receive pin to INPUT
    *rReg &= ~rBit;         // Set receive pin to INPUT
    *sOut |= sBit;          // Set send pin HIGH
    
    // Measure timing
    timeout = 0;
    while (!(*rIn & rBit)) {
        if (timeout++ > CS_Timeout_Millis * 1000) {
            // Restore pin states and return
            *sOut &= ~sBit;     // Set send pin LOW
            interrupts();       // Re-enable interrupts
            return 0;           // Indicate timeout
        }
    }
    
    // Send pin LOW
    *sOut &= ~sBit;         // Set send pin LOW
    reg = timeout;
    interrupts();           // Re-enable interrupts
    return 1;               // Indicate successful cycle
}
