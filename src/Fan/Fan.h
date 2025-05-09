#include <Arduino.h>

namespace SmartAirControl {
    class Fan {
        public:
            Fan();
            void loop();
            void setup();
            float getRpm();
            void setRpmPercent(int percent);

        private:
            int fanPwmPin;
            int tachPin;
            volatile int pulseCount;
            volatile unsigned long lastPulse;
            float rpmTable[11];
            int dutyTable[11];
            float maxRpm;
            int currentPercent;
            unsigned long lastRpmTime;

            int getInterpolatedDuty(int percent);
            void IRAM_ATTR countPulse();
    };
}