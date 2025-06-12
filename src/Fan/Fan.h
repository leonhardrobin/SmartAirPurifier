#include <Arduino.h>

namespace SmartAirControl {
    class Fan {
        public:
            Fan(int fanPwmPin, int tachPin);
            void setup();
            int getRpm();
            void setRpmPercent(int percent);
            float getRpmPercent();

        private:
            static const int N = 11;
            int fanPwmPin;
            int tachPin;
            static volatile int pulseCount;
            static volatile unsigned long lastPulse;
            int rpmTable[N];
            int dutyTable[N];
            float maxRpm;
            int currentPercent;
            unsigned long lastRpmTime;

            int getInterpolatedDuty(int percent);
            static void IRAM_ATTR countPulse();
    };
}