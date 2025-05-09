#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme;

void setupy() {
  Serial.begin(115200);
  while (!Serial);

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loopy() {
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("BME680 failed to begin reading!"));
    return;
  }

  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  if (!bme.endReading()) {
    Serial.println(F("BME680 failed to complete reading!"));
    return;
  }

  Serial.print(F("Temperature = "));
  Serial.print(bme.temperature, 2);
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(bme.pressure / 100.0, 2);
  Serial.println(" hPa");

  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity, 2);
  Serial.println(" %RH");

  Serial.print(F("Gas = "));
  Serial.print(bme.gas_resistance / 1000.0, 2);
  Serial.println(" KOhm");

  Serial.print(F("Altitude = "));
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA), 2);
  Serial.println(" m");

  Serial.println();
  delay(2000);
}