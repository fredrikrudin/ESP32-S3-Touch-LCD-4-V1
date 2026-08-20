#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "Config.h"
#include "VictronDecrypt.h"

BLEScan* pBLEScan;
unsigned long last_ble_scan = 0;
bool is_scanning = false;

// --- DYNAMISK PARSING AV DEKRYPTERAD DATA ---
void parseDecryptedData(uint8_t* decrypted, size_t len, int deviceType) {
    if (deviceType == 1) { // Exempel: SmartShunt
        // Victron SmartShunt layout (Rå-bytes mappas till spänning och ström)
        // Spänning ligger ofta i byte 0-1 (skalat med 0.01V eller 0.1V beroende på modell)
        int16_t raw_volt = (decrypted[1] << 8) | decrypted[0];
        int32_t raw_curr = (decrypted[4] << 16) | (decrypted[3] << 8) | decrypted[2]; // 3-bytes strömstyrka
        
        shunt.voltage = raw_volt / 100.0;
        shunt.current = raw_curr / 1000.0; // mA till A
        shunt.soc = decrypted[5] / 2.0;    // SoC skickas ofta ut i halva procent (0-200)
        
        Serial.printf("[Shunt] Volt: %.2fV, Ström: %.2fA, SoC: %.1f%%\n", shunt.voltage, shunt.current, shunt.soc);
    } 
    else if (deviceType == 2) { // Exempel: MPPT Solcellsregulator
        int16_t raw_volt = (decrypted[1] << 8) | decrypted[0];
        int16_t raw_power = (decrypted[3] << 8) | decrypted[2]; // Watt ut från paneler
        
        mppt.voltage = raw_volt / 100.0;
        // Vi simulerar strömmen genom P/U
        mppt.current = (raw_volt > 0) ? (raw_power / mppt.voltage) : 0; 
        
        Serial.printf("[MPPT] Panel Power: %d W, Laddspänning: %.2fV\n", raw_power, mppt.voltage);
    }
}

// --- CALLBACK: KÖRS SÅ FORT ESP32 FÅNGAR ETT BLUETOOTH-PAKET ---
class VictronAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String foundMac = advertisedDevice.getAddress().toString().c_str();
        foundMac.toUpperCase(); // Säkerställ matchning mot minnet

        // Kontrollera om enheten är aktiverad och matchar SmartShunt
        if (shunt.enabled && foundMac == shunt.mac) {
            if (advertisedDevice.haveManufacturerData()) {
                std::string mData = advertisedDevice.getManufacturerData();
                uint8_t* rawData = (uint8_t*)mData.data();
                
                // Kontrollera Victrons tillverkarsignatur (0x02FF eller rekordtyp)
                uint8_t decrypted[16];
                if (decryptVictronPayload(rawData, mData.length(), shunt.key, decrypted)) {
                    parseDecryptedData(decrypted, mData.length() - 5, 1); // 1 = Shunt
                }
            }
        }
        
        // Kontrollera om enheten är aktiverad och matchar MPPT
        if (mppt.enabled && foundMac == mppt.mac) {
            if (advertisedDevice.haveManufacturerData()) {
                std::string mData = advertisedDevice.getManufacturerData();
                uint8_t* rawData = (uint8_t*)mData.data();
                
                uint8_t decrypted[16];
                if (decryptVictronPayload(rawData, mData.length(), mppt.key, decrypted)) {
                    parseDecryptedData(decrypted, mData.length() - 5, 2); // 2 = MPPT
                }
            }
        }
    }
};

void init_ble_scanner() {
    BLEDevice::init("");
    pBLEScan = BLEDevice::getBLEScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new VictronAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(false); // Victron kräver endast passiv skanning (sparar ström)
    pBLEScan->setInterval(150);     // Tätare intervall för att inte missa paket
    pBLEScan->setWindow(140);
}

// --- DEN UTÖKADE OCH ICKE-BLOCKERANDE LOOP-FUNKTIONEN ---
void loop() {
    lv_timer_handler(); // Kör LVGL-gränssnittet (bör köras var 5:e ms)
    delay(5);
    update_display_dimming(); // Hantera skärmens nattläge och belysningstimer

    // Hantera BLE-skanning asynkront baserat på ditt inställda tidsintervall (t.ex. var 2:e sekund)
    unsigned long current_millis = millis();
    if (current_millis - last_ble_scan > (update_interval * 1000)) {
        
        // Vi kör en kort, icke-blockerande passiv skanning på 1 sekund
        // Det förhindrar att skärmen laggar eller fryser under tiden ESP32 lyssnar efter Bluetooth
        pBLEScan->start(1, false); 
        pBLEScan->clearResults(); // Töm cachen för att spara RAM-minne
        
        last_ble_scan = millis();
    }
}
