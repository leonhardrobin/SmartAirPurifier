#include "BME.h"

namespace SmartAirControl {

    BME::BME(u_int8_t tempOversampling,
            u_int8_t humidityOversampling,
            u_int8_t pressureOversampling,
            u_int8_t IIRFilterSize,
            int gasHeaterTemp,
            int gasHeaterDuration,
            float seaLevelPressure_hPa)
          : bme(Adafruit_BME680()),
          tempOversampling(tempOversampling),
          humidityOversampling(humidityOversampling),
          pressureOversampling(pressureOversampling),
          IIRFilterSize(IIRFilterSize),
          gasHeaterTemp(gasHeaterTemp),
          gasHeaterDuration(gasHeaterDuration),
          sealevelPressure_hPa(seaLevelPressure_hPa)
            {}

    void BME::setup() {
      valid = bme.begin();
      if (!valid) {
          Serial.println(F("[BME680] Could not find a valid BME680 sensor, check wiring!"));
      }

      // Set up oversampling and filter initialization
      bme.setTemperatureOversampling(tempOversampling);
      bme.setHumidityOversampling(humidityOversampling);
      bme.setPressureOversampling(pressureOversampling);
      bme.setIIRFilterSize(IIRFilterSize);
      bme.setGasHeater(gasHeaterTemp, gasHeaterDuration);
    }

    bool BME::isValid() {
      return valid;
    }

    BMEData BME::read() {
      BMEData bmeData = BMEData();
      unsigned long endTime = bme.beginReading();
      if (endTime == 0) {
        Serial.println(F("[BME680] Failed to begin reading!"));
        return bmeData;
      }

      Serial.print(F("[BME680] Reading started at "));
      Serial.print(millis());
      Serial.print(F(" and will finish at "));
      Serial.println(endTime);

      if (!bme.endReading()) {
        Serial.println(F("[BME680] Failed to complete reading!"));
        return bmeData;
      }

      bmeData.temperature = bme.temperature;
      bmeData.pressure = bme.pressure / 100.0;
      bmeData.humidity = bme.humidity;
      bmeData.altitude = bme.readAltitude(sealevelPressure_hPa);
      bmeData.gasResistance = bme.gas_resistance / 1000.0;

      printSensorData(bmeData);
      
      return bmeData;
    }

    void BME::printSensorData(BMEData& bmeData) {
      Serial.println(F("[BME680]"));
      Serial.println(F("---------------------------------------"));
      Serial.print(F("Temperature: "));
      Serial.print(bmeData.temperature);
      Serial.println(F(" °C"));
      Serial.print(F("Pressure: "));
      Serial.print(bmeData.pressure);
      Serial.println(F(" hPa"));
      Serial.print(F("Humidity: "));
      Serial.print(bmeData.humidity);
      Serial.println(F(" %"));
      Serial.print(F("Altitude: "));
      Serial.print(bmeData.altitude);
      Serial.println(F(" m"));
      Serial.print(F("Gas Resistance: "));
      Serial.print(bmeData.gasResistance);
      Serial.println(F(" KOhm"));
    }

}