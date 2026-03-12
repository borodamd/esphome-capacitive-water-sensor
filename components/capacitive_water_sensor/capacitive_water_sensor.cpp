/*
  CapacitiveSensor.cpp v.04 - Capacitive Sensing Library
  https://github.com/PaulStoffregen/CapacitiveSensor
  Copyright (c) 2009 Paul Bagder  All right reserved.
*/

#include "CapacitiveSensor.h"

// Constructor
CapacitiveSensor::CapacitiveSensor(uint8_t sendPin, uint8_t receivePin)
{
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
    rReg = portOutputRegister(digitalPinToPort(receivePin));
    rIn = portInputRegister(digitalPinToPort(receivePin));
    rOut = rReg;
    rReg = portModeRegister(digitalPinToPort(receivePin));
    
    // reset pin modes
    *rReg &= ~rBit;  // Set receive pin to INPUT
    *sOut &= ~sBit;  // Set send pin LOW
    pinMode(sendPin, OUTPUT);  // Make send pin an output
    pinMode(receivePin, INPUT); // Make receive pin an input
}

long CapacitiveSensor::capacitiveSensor(uint8_t samples)
{
    total = 0;
    if (samples == 0) return 0;
    for (uint8_t i = 0; i < samples; i++) {
        total += capacitiveSensorRaw(1);
    }
    
    // Only calibrate if the calibration interval has passed
    // ИСПРАВЛЕНО: явное приведение типов для abs()
    if ( (millis() - lastCal > CS_AutocaL_Millis) && labs((long)(total - leastTotal)) < (long)(0.10f * (float)leastTotal) ) {
        leastTotal = total;
        lastCal = millis();
    }
    
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
            total1 += reg;
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

int CapacitiveSensor::CycleThis()
{
    noInterrupts();
    *rOut &= ~rBit;
    *rReg |= rBit;
    delayMicroseconds(10);
    *rReg &= ~rBit;
    *sOut |= sBit;
    
    timeout = 0;
    while (!(*rIn & rBit)) {
        if (timeout++ > CS_Timeout_Millis * (loopTimingFactor / 100)) {
            *sOut &= ~sBit;
            interrupts();
            return 0;
        }
    }
    
    *sOut &= ~sBit;
    reg = timeout;
    interrupts();
    return 1;
}
