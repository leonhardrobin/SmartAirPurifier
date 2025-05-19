#include <Arduino.h>
#include <HardwareSerial.h>
#include <Adafruit_PM25AQI.h>

namespace SmartAirControl {

    class PMS {
        public:
            PMS(int rxPin, int txPin, unsigned long serialBaud, SerialConfig serialConfig);
            PM25_AQI_Data read();
            void setup();
        private:
            PM25_AQI_Data data;
            HardwareSerial pmsSerial;
            unsigned long serialBaud;
            SerialConfig serialConfig;
            int rxPin;
            int txPin;
            Adafruit_PM25AQI pms;
    };
}