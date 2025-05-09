#include <Arduino.h>
#include <HardwareSerial.h>

namespace SmartAirControl {

    class PMS {
        public:
            PMS(int rxPin, int txPin, unsigned long serialBaud, SerialConfig serialConfig);
            void read();
            void setup();
            uint16_t getPm2_5();
            uint16_t getPm10();

        private:
            uint16_t pm2_5;
            uint16_t pm10;
            HardwareSerial pmsSerial;
            unsigned long serialBaud;
            SerialConfig serialConfig;
            int rxPin;
            int txPin;
    };
}