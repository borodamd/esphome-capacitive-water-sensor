#include "CapacitiveSensor.h"

CapacitiveSensor::CapacitiveSensor(uint8_t sendPin, uint8_t receivePin) {
    uint8_t sPort = digitalPinToPort(sendPin);
    uint8_t rPort = digitalPinToPort(receivePin);
    if (sPort == NOT_A_PORT || rPort == NOT_A_PORT) { error = -1; return; }
    sBit = digitalPinToBitMask(sendPin);
    sReg = portModeRegister(sPort);
    sOut = portOutputRegister(sPort);
    rBit = digitalPinToBitMask(receivePin);
    rReg = portModeRegister(rPort);
    rOut = portOutputRegister(rPort);
    rIn  = portInputRegister(rPort);
    lastCal = millis();
}

long CapacitiveSensor::capacitiveSensorRaw(uint8_t samples) {
    long total = 0; // Перенесли внутрь
    if (error < 0) return -1;
    for (uint8_t i = 0; i < samples; i++) {
        *sOut &= ~sBit; *rReg |= rBit; *rOut &= ~rBit; *rReg &= ~rBit;
        while (*rIn & rBit && (millis() - lastCal < CS_Timeout_Millis));
        *sOut |= sBit;
        int cycle = 0;
        while (!(*rIn & rBit) && (cycle < CS_Timeout_Millis)) { cycle++; }
        if (cycle >= CS_Timeout_Millis) return -2;
        total += cycle;
    }
    return total;
}

long CapacitiveSensor::capacitiveSensor(uint8_t samples) {
    long total = capacitiveSensorRaw(samples);
    if (total < 0) return total;
    // Исправленная строка с приведением типа для abs()
    if ((millis() - lastCal > CS_AutocaL_Millis) && std::abs((long)(total - leastTotal)) < (int)(.10 * (float)leastTotal)) {
        leastTotal = 0x0FFFFFFFL; lastCal = millis();
    }
    if (total < leastTotal) leastTotal = total;
    return total - leastTotal;
}

void CapacitiveSensor::set_CS_Timeout_Millis(unsigned long timeout_millis) { CS_Timeout_Millis = timeout_millis; }
void CapacitiveSensor::set_CS_AutocaL_Millis(unsigned long autoCal_millis) { CS_AutocaL_Millis = autoCal_millis; }
unsigned long total;
