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
    #endif
    
    bme.startReading();

    // do other work while waiting for the BME680 to finish
    
    // read PMS data
    uint16_t pm2_5 = pms.getPm2_5();
    uint16_t pm10 = pms.getPm10();
    Serial.print(F("[APP] PMS PM2.5: "));
    Serial.println(pm2_5);
    Serial.print(F("[APP] PMS PM10: "));
    Serial.println(pm10);

    // delay for BME680 to finish reading
    delay(1000); // adjust as needed

    SmartAirControl::BMEData bmeData = bme.endReading();

    Serial.print(F("[APP] BME680 Temperature: "));
    Serial.print(bmeData.temperature);
    Serial.println(F(" °C"));
    Serial.print(F("[APP] BME680 Pressure: "));
    Serial.print(bmeData.pressure);
    Serial.println(F(" hPa"));
    Serial.print(F("[APP] BME680 Humidity: "));
    Serial.print(bmeData.humidity);
    Serial.println(F(" %"));
    Serial.print(F("[APP] BME680 Altitude: "));
    Serial.print(bmeData.altitude);
    Serial.println(F(" m"));
    Serial.print(F("[APP] BME680 Gas Resistance: "));
    Serial.print(bmeData.gasResistance);
    Serial.println(F(" KOhm"));

    #if USE_LORAWAN == 1

    // Create a JSON object
    JsonDocument doc;
    doc["temperature"] = bmeData.temperature;
    doc["pressure"] = bmeData.pressure;
    doc["humidity"] = bmeData.humidity;
    doc["altitude"] = bmeData.altitude;
    doc["gasResistance"] = bmeData.gasResistance;
    doc["pm2_5"] = pm2_5;
    doc["pm10"] = pm10;

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
    #endif
    pms.read();
}

// Does it respond to a UBX-MON-VER request?
// uint8_t ubx_mon_ver[] = { 0xB5,0x62,0x0A,0x04,0x00,0x00,0x0E,0x34 };