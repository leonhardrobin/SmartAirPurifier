/*

This demonstrates how to save the join information in to permanent memory
so that if the power fails, batteries run out or are changed, the rejoin
is more efficient & happens sooner due to the way that LoRaWAN secures
the join process - see the wiki for more details.

This is typically useful for devices that need more power than a battery
driven sensor - something like a air quality monitor or GPS based device that
is likely to use up it's power source resulting in loss of the session.

The relevant code is flagged with a ##### comment

Saving the entire session is possible but not demonstrated here - it has
implications for flash wearing and complications with which parts of the
session may have changed after an uplink. So it is assumed that the device
is going in to deep-sleep, as below, between normal uplinks.

Once you understand what happens, feel free to delete the comments and
Serial.prints - we promise the final result isn't that many lines.

*/

#if !defined(ESP32)
#pragma error("This is not the example your device is looking for - ESP32 only")
#endif

#include <Preferences.h>

#if USE_LORAWAN == 1
RTC_DATA_ATTR uint16_t bootCount = 0;
#include "LoRa/LoRAWAN.hpp"
#include <ArduinoJson.h>
#endif
#include "BME/BME.h"
#include "PMS/PMS.h"
#include "Fan/Fan.h"

#if USE_LORAWAN == 1
static SmartAirControl::LoRaWAN<RADIOLIB_LORA_MODULE> loRaWAN(RADIOLIB_LORA_REGION,
                                                   RADIOLIB_LORAWAN_JOIN_EUI,
                                                   RADIOLIB_LORAWAN_DEV_EUI,
                                                   (uint8_t[16]) {RADIOLIB_LORAWAN_APP_KEY},
#ifdef RADIOLIB_LORAWAN_NWK_KEY
                                                   (uint8_t[16]) {RADIOLIB_LORAWAN_NWK_KEY},
#else
                                                   nullptr,
#endif
                                                   RADIOLIB_LORA_MODULE_BITMAP);

#endif

static SmartAirControl::BME bme(BME680_OS_8X, 
                    BME680_OS_2X, 
                    BME680_OS_4X,
                    BME680_FILTER_SIZE_3, 
                    320, 150,
                    1013.25);
static SmartAirControl::PMS pms(16, 17, 9600, SERIAL_8N1);
static SmartAirControl::Fan fan(13, 12);

#if USE_LORAWAN == 1
// abbreviated version from the Arduino-ESP32 package, see
// https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/deepsleep.html
// for the complete set of options
void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println(F("Wake from sleep"));
    } else {
        Serial.print(F("Wake not caused by deep sleep: "));
        Serial.println(wakeup_reason);
    }

    Serial.print(F("Boot count: "));
    Serial.println(++bootCount); // increment before printing
}

void gotoSleep(uint32_t seconds) {
    loRaWAN.goToSleep();

    Serial.println("[APP] Go to sleep");
    Serial.println();

    esp_sleep_enable_timer_wakeup(seconds * 1000UL * 1000UL); // function uses uS
    esp_deep_sleep_start();

    Serial.println(F("\n\n### Sleep failed, delay of 5 minutes & then restart ###\n"));
    delay(5UL * 60UL * 1000UL);
    ESP.restart();
}
#endif

class Data {
    public:
        Data(PM25_AQI_Data pmsData, SmartAirControl::BMEData bmeData, float FanRpm) : pmsData(pmsData), bmeData(bmeData), FanRpm(FanRpm) {}
        PM25_AQI_Data pmsData;
        SmartAirControl::BMEData bmeData;
        float FanRpm;
};

Data readSensors() {
    PM25_AQI_Data pmsData = pms.read();
    SmartAirControl::BMEData bmeData = bme.read();
    float FanRpm = fan.getRpm();
    return Data(pmsData, bmeData, FanRpm);
}

void adjustFanSpeed(Data data) {
    // Simple algorithm to adjust fan speed based on air quality and temperature

    float gas = data.bmeData.gasResistance;
    float pm1 = data.pmsData.particles_10um;
    float pm25 = data.pmsData.particles_25um;
    float pm10 = data.pmsData.particles_100um;
    float temp = data.bmeData.temperature;

    // Normalize sensor values (example thresholds, adjust as needed)
    float gasScore = gas < 10000 ? 1.0 : (gas < 20000 ? 0.5 : 0.0); // lower gas resistance = worse air
    float pmScore = (pm1 + pm25 + pm10) / 3.0;
    float pmNorm = pmScore < 10 ? 0.0 : (pmScore < 35 ? 0.5 : 1.0); // higher PM = worse air
    float tempScore = temp > 30 ? 1.0 : (temp > 25 ? 0.5 : 0.0); // higher temp = higher speed

    // Weighted sum (tune weights as needed)
    float score = 0.2 * gasScore + 0.7 * pmNorm + 0.1 * tempScore;

    float fanPercent = score * 100;

    fan.setRpmPercent(fanPercent);

    Serial.print(F("[APP] Adjusting fan speed to "));
    Serial.print(fanPercent);
    Serial.println(F("% based on air quality and temperature."));
}

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ;        // wait for serial to be initalised
    delay(2000); // give time to switch to the serial monitor

    #if USE_LORAWAN == 1
    print_wakeup_reason();
    #endif
    
    Serial.println(F("Setup"));

    #if USE_LORAWAN == 1
    loRaWAN.setup(bootCount);
    #endif
    
    bme.setup();
    pms.setup();
    fan.setup();
    fan.setRpmPercent(100);

    #if USE_LORAWAN == 1
    loRaWAN.setDownlinkCB([](uint8_t fPort, uint8_t* downlinkPayload, std::size_t downlinkSize) {
        Serial.print(F("[APP] Payload: fPort="));
        Serial.print(fPort);
        Serial.print(", ");
        SmartAirControl::arrayDump(downlinkPayload, downlinkSize);
    });
    Serial.println(F("[APP] Aquire data and construct LoRaWAN uplink"));

    std::string uplinkPayload = RADIOLIB_LORAWAN_PAYLOAD;
    uint8_t fPort = 2;

    Data sensorData = readSensors();

    // Create a JSON object
    JsonDocument doc;
    doc["temperature"] = sensorData.bmeData.temperature;
    doc["pressure"] = sensorData.bmeData.pressure;
    doc["humidity"] = sensorData.bmeData.humidity;
    doc["altitude"] = sensorData.bmeData.altitude;
    doc["gasResistance"] = sensorData.bmeData.gasResistance;

    // Serialize to string
    String jsonString;
    serializeJson(doc, jsonString);

    // Assign to uplink payload
    uplinkPayload = std::string(jsonString.c_str());
    

    loRaWAN.setUplinkPayload(fPort, uplinkPayload);
    #endif
}

void loop() {
    #if USE_LORAWAN == 1
    loRaWAN.loop();
    #else
    Data data = readSensors();
    adjustFanSpeed(data);
    delay(5000);
    #endif
}

// Does it respond to a UBX-MON-VER request?
// uint8_t ubx_mon_ver[] = { 0xB5,0x62,0x0A,0x04,0x00,0x00,0x0E,0x34 };