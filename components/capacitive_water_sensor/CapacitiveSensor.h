#ifndef CapacitiveSensor_h
#define CapacitiveSensor_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class CapacitiveSensor {
public:
    CapacitiveSensor(uint8_t sendPin, uint8_t receivePin);
    long capacitiveSensorRaw(uint8_t samples);
    long capacitiveSensor(uint8_t samples);
    void set_CS_Timeout_Millis(unsigned long timeout_millis);
    void set_CS_AutocaL_Millis(unsigned long autoCal_millis);

private:
    uint8_t  error;
    unsigned long  leastTotal;
    unsigned long  loopTimingFactor;
    unsigned long  lastCal;
    unsigned long  CS_Timeout_Millis;
    unsigned long  CS_AutocaL_Millis;
    uint8_t sBit;   // send pin's ports and bitmask
    volatile uint8_t *sReg;
    volatile uint8_t *sOut;
    uint8_t rBit;    // receive pin's ports and bitmask
    volatile uint8_t *rReg;
    volatile uint8_t *rOut;
    volatile uint8_t *rIn;
    int DirectInput(uint8_t pin);
};
#endif
