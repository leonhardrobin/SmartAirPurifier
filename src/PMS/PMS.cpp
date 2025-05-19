#include "PMS.h"

namespace SmartAirControl {

  SmartAirControl::PMS::PMS(int rxPin, int txPin, unsigned long serialBaud, SerialConfig serialConfig) 
      : pmsSerial(HardwareSerial(2)), serialBaud(serialBaud), serialConfig(serialConfig), rxPin(rxPin), txPin(txPin), pms(Adafruit_PM25AQI()) {
  }

  void SmartAirControl::PMS::setup() {
    pmsSerial.begin(9600, SERIAL_8N1, 16, 17);
      
    if (!pms.begin_UART(&pmsSerial)) {
      Serial.println("[PMS5003] Could not find PMS5003 sensor, check wiring!");
      while (1) {
        delay(10);
      }
    }
  }
    
  PM25_AQI_Data SmartAirControl::PMS::read() {
    data = PM25_AQI_Data();
    if (!pms.read(&data)) {
      Serial.println("[PMS5003] Could not read from PMS5003 sensor!");
      return data;
    }

    printSensorData();

    return data;
  }

  void PMS::printSensorData() {
    Serial.println("[PMS5003]");
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
    Serial.print("\tPM 2.5: "); Serial.print(data.pm25_standard);
    Serial.print("\tPM 10.0: "); Serial.print(data.pm100_standard);
    Serial.println(" ug/m3");

    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
    Serial.print("\tPM 2.5: "); Serial.print(data.pm25_env);
    Serial.print("\tPM 10.0: "); Serial.print(data.pm100_env);
    Serial.println(" ug/m3");

    Serial.println("---------------------------------------");
    Serial.print("Particles > 0.3um / 0.1L air: "); Serial.println(data.particles_03um);
    Serial.print("Particles > 0.5um / 0.1L air: "); Serial.println(data.particles_05um);
    Serial.print("Particles > 1.0um / 0.1L air: "); Serial.println(data.particles_10um);
    Serial.print("Particles > 2.5um / 0.1L air: "); Serial.println(data.particles_25um);
    Serial.print("Particles > 5.0um / 0.1L air: "); Serial.println(data.particles_50um);
    Serial.print("Particles > 10.0 um / 0.1L air: "); Serial.println(data.particles_100um);
    Serial.println("---------------------------------------");
  }

}