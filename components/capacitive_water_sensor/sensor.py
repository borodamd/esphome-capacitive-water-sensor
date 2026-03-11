// components/capacitive_water_sensor/capacitive_sensor_impl.h
#pragma once

#include <Arduino.h>

class SimpleCapacitiveSensor {
private:
    int sendPin;
    int receivePin;
    unsigned long timeout;

public:
    SimpleCapacitiveSensor(int send, int receive) {
        sendPin = send;
        receivePin = receive;
        timeout = 500000; // 500 ms в микросекундах
    }
    
    void set_CS_Timeout_Millis(unsigned long ms) {
        timeout = ms * 1000L;
    }
    
    void set_CS_AutocaL_Millis(unsigned long) {
        // Отключаем автокалибровку
    }
    
    long capacitiveSensorRaw(unsigned int samples) {
        long total = 0;
        pinMode(receivePin, INPUT);
        pinMode(sendPin, OUTPUT);
        
        for (unsigned int i = 0; i < samples; i++) {
            digitalWrite(sendPin, HIGH);
            unsigned long start = micros();
            unsigned long end = start;
            
            while (digitalRead(receivePin) == LOW && (end - start) < timeout) {
                end = micros();
            }
            
            total += (end - start);
            
            digitalWrite(sendPin, LOW);
            delayMicroseconds(10);
        }
        
        pinMode(sendPin, INPUT);
        
        if (total == 0) return -2; // Короткое замыкание
        return total / samples;
    }
};
