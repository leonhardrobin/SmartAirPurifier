#include "PMS.h"

namespace SmartAirControl {

  SmartAirControl::PMS::PMS(int rxPin, int txPin, unsigned long serialBaud, SerialConfig serialConfig) 
      : pmsSerial(HardwareSerial(2)), serialBaud(serialBaud), serialConfig(serialConfig), rxPin(rxPin), txPin(txPin) {
  }

  void SmartAirControl::PMS::setup() {
      pmsSerial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  }
    
  void SmartAirControl::PMS::read() {
    if (pmsSerial.available() >= 32) { // PMS5003 sends 32-byte frames
      uint8_t buffer[32];
      pmsSerial.readBytes(buffer, 32);

      // Check header
      if (buffer[0] == 0x42 && buffer[1] == 0x4D) {
        pm2_5 = (buffer[12] << 8) | buffer[13];
        pm10  = (buffer[14] << 8) | buffer[15];
      }
    }
  }

  uint16_t SmartAirControl::PMS::getPm2_5() {
      return pm2_5;
  }

  uint16_t SmartAirControl::PMS::getPm10() {
      return pm10;
  }

}