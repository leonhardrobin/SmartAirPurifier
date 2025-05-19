#include <Arduino.h>

namespace SmartAirControl {
    class Fan {
        public:
            Fan(int fanPwmPin, int tachPin);
            void setup();
            float getRpm();
            void setRpmPercent(int percent);

        private:
            static const int N = 11;
            int fanPwmPin;
            int tachPin;
            static volatile int pulseCount;
            static volatile unsigned long lastPulse;
            float rpmTable[N];
            int dutyTable[N];
            float maxRpm;
            int currentPercent;
            unsigned long lastRpmTime;

            int getInterpolatedDuty(int percent);
            static void IRAM_ATTR countPulse();
    };
}