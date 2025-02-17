#ifndef Ultrasonic_H
#define Ultrasonic_H

#include <chrono>
#include <thread>

#define CM 58
#define MM 5.8
#define NUM_READINGS 10
#define MAX_DISTANCE 80

long pulseIn(int pin, int value, unsigned long timeout = 1000000) {
    unsigned long startTime = micros();
    unsigned long duration = 0;

    while (digitalRead(pin) != value) {
        if (micros() - startTime > timeout)
            return 0;
    }

    startTime = micros();

    while (digitalRead(pin) == value) {
        if (micros() - startTime > timeout)
            return 0;
    }

    duration = micros() - startTime;

    return duration;
}

class Ultrasonic {
private:
    uint8_t trig, echo;
    unsigned long timeOut;
    unsigned int readings[NUM_READINGS] = { 0 };
    size_t readIndex = 0;
    unsigned int total = 0;
    unsigned int lastValidDistance = 0;

public:
    Ultrasonic(uint8_t _trigPin, uint8_t _echoPin, unsigned long _timeOut = 20000UL)
        : trig(_trigPin), echo(_echoPin), timeOut(_timeOut) {
        pinMode(trig, OUTPUT);
        pinMode(echo, INPUT);
    }

    unsigned int getDistanceAndAverage(int type) {
        digitalWrite(trig, LOW);
        delayMicroseconds(2);
        digitalWrite(trig, HIGH);
        delayMicroseconds(10);
        digitalWrite(trig, LOW);
        unsigned int distance = pulseIn(echo, HIGH, timeOut) / type;
        if (type == MM) {
            if (distance > (MAX_DISTANCE * 10))
                return lastValidDistance;
        }
        else {
            if (distance > MAX_DISTANCE)
                return lastValidDistance;
        }

        //if (readIndex > 0 && distance > readings[(readIndex - 1 + NUM_READINGS) % NUM_READINGS] * 2)
        //    return 0;

        total -= readings[readIndex];
        readings[readIndex] = distance;
        total += readings[readIndex];
        readIndex = (readIndex + 1) % NUM_READINGS;

        //return total / NUM_READINGS;
        lastValidDistance = total / NUM_READINGS;
        return lastValidDistance;
    }
};

#endif // !Ultrasonic_update_H
