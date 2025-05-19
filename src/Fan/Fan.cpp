#include "Fan.h"
#include <Arduino.h>

namespace SmartAirControl {

// Define static member variables
volatile int Fan::pulseCount = 0;
volatile unsigned long Fan::lastPulse = 0;

    Fan::Fan(int fanPwmPin, int tachPin)
        : fanPwmPin(fanPwmPin), tachPin(tachPin),
          rpmTable{   0,   780,  1140,  1440,  1740,  2730,  6720,  9960, 12930, 14520, 12570 },
          dutyTable{ 255,   230,   204,   179,   153,   128,   102,    77,    51,    26,     0 },
          maxRpm(14520.0), currentPercent(0), lastRpmTime(0) {
    }

    void Fan::setup() {
        pinMode(fanPwmPin, OUTPUT);
        analogWrite(fanPwmPin, 255); 

        pinMode(tachPin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(tachPin), countPulse, FALLING);
    }

    float Fan::getRpm() {
        noInterrupts();
            int count = pulseCount;
            pulseCount = 0;
        interrupts();

        return (count / 2.0) * 60.0;
    }
    
    void Fan::setRpmPercent(int percent) {
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;

        if (currentPercent != percent) {
            currentPercent = percent;
            int duty = getInterpolatedDuty(percent);
            analogWrite(fanPwmPin, duty);
        }
    }
    

    int Fan::getInterpolatedDuty(int percent) {
        float desiredRpm = percent / 100.0 * maxRpm;

        // unterhalb Minimum
        if (desiredRpm <= rpmTable[0]) return dutyTable[0];
        // oberhalb Maximum
        if (desiredRpm >= rpmTable[N-1]) return dutyTable[N-1];

        // Segment suchen und interpolieren
        for (int i = 0; i < N - 1; i++) {
            if (desiredRpm <= rpmTable[i+1]) {
            float frac = (desiredRpm - rpmTable[i]) / (rpmTable[i+1] - rpmTable[i]);
            float d    = dutyTable[i] + frac * (dutyTable[i+1] - dutyTable[i]);
            return int(d + 0.5);  // aufrunden
            }
        }
        return dutyTable[N-1];
    }

    void IRAM_ATTR Fan::countPulse() {
        unsigned long now = micros();
        if (now - lastPulse > 1000) {   // 1 ms Debounce
            pulseCount++;
            lastPulse = now;
        }
    }
}