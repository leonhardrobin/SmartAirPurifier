#ifndef BME_H
#define BME_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

namespace SmartAirControl {
    class BMEData {
        public:
            BMEData()
                : temperature(0.0),
                  pressure(0.0),
                  humidity(0.0),
                  altitude(0.0),
                  gasResistance(0.0) {}

            float temperature;  /** Temperature in degrees celsius */
            float pressure;     /** Pressure in hPa */
            float humidity;     /** Humidity in % */
            float altitude;     /** Altitude in meters */
            float gasResistance;/** Gas resistance in KOhms */
    };

    class BME {
        private:
            Adafruit_BME680 bme;
            BMEData bmeData;
            uint8_t tempOversampling;
            uint8_t humidityOversampling;
            uint8_t pressureOversampling;
            uint8_t IIRFilterSize;
            int gasHeaterTemp;
            int gasHeaterDuration;
            float sealevelPressure_hPa;
            bool valid = false;
        public:
            BME(u_int8_t tempOversampling,
                u_int8_t humidityOversampling,
                u_int8_t pressureOversampling,
                u_int8_t IIRFilterSize,
                int gasHeaterTemp,
                int gasHeaterDuration,
                float seaLevelPressure_hPa);

            void setup();
            BMEData read();
            bool isValid();
            void printSensorData(BMEData& bmeData);
    };

}

#endif // BME_H