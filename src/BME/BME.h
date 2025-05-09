#ifndef BME_H
#define BME_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

namespace SmartAirControl {
    class BMEData {
        public:
            BMEData(float temperature = 0.0,
                    float pressure = 0.0,
                    float humidity = 0.0,
                    float altitude = 0.0,
                    float gasResistance = 0.0) {
                this->temperature = temperature;
                this->pressure = pressure;
                this->humidity = humidity;
                this->altitude = altitude;
                this->gasResistance = gasResistance;
            }

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
            void startReading();
            BMEData endReading();
            bool isValid();
    };

}

#endif // BME_H