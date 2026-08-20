// Global variabel för skärmens larm-label
lv_obj_t * lbl_inv_data_page;

class VictronAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String foundMac = advertisedDevice.getAddress().toString().c_str();
        foundMac.toUpperCase();

        // 1. Hantera SmartShunt (Type 1)
        if (shunt.enabled && foundMac == shunt.mac) {
            if (advertisedDevice.haveManufacturerData()) {
                std::string mData = advertisedDevice.getManufacturerData();
                uint8_t* rawData = (uint8_t*)mData.data();
                uint8_t decrypted[16];
                if (decryptVictronPayload(rawData, mData.length(), shunt.key, decrypted)) {
                    parseDecryptedData(decrypted, mData.length() - 5, 1);
                }
            }
        }
        
        // 2. Hantera MPPT (Type 2) ... [Samma som tidigare]

        // --- 3. NYTT: HANTERA PHOENIX INVERTER (Type 3) ---
        if (inverter.enabled && foundMac == inverter.mac) {
            if (advertisedDevice.haveManufacturerData()) {
                std::string mData = advertisedDevice.getManufacturerData();
                uint8_t* rawData = (uint8_t*)mData.data();
                
                // Kontrollera Victrons tillverkarsignatur (0x02FF) och Record Type (0x03 för Inverter)
                if (rawData[0] == 0xFF && rawData[1] == 0x02 && rawData[2] == 0x03) {
                    uint8_t decrypted[16];
                    if (decryptVictronPayload(rawData, mData.length(), inverter.key, decrypted)) {
                        parseDecryptedData(decrypted, mData.length() - 5, 3); // 3 = Inverter
                        
                        // Uppdatera skärmens dedikerade Inverter-sida omedelbart vid mottagning
                        if (lbl_inv_data_page != NULL) {
                            String alarmText = parseInverterAlarms(inverter.alarm_code);
                            lv_label_set_text_fmt(lbl_inv_data_page, 
                                "Effekt: %d W\nIn-Volt: %.2f V\nStatus: %s\n\n⚠️ %s", 
                                inverter.ac_watt, 
                                inverter.battery_voltage, 
                                get_inverter_state_str(inverter.state_code),
                                alarmText.c_str());
                        }
                    }
                }
            }
        }
    }
};

// I din befintliga init_lvgl_interface(), där "tab_inverter" skapas:
void update_lvgl_inverter_tab_setup() {
    if (inverter.enabled && tab_inverter != NULL) {
        // Skapa en textruta på Inverter-fliken för att rymma live-data och larm
        lbl_inv_data_page = lv_label_create(tab_inverter);
        lv_label_set_text(lbl_inv_data_page, "Väntar på BLE-data...");
        lv_obj_set_style_text_color(lbl_inv_data_page, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(lbl_inv_data_page, &lv_font_montserrat_16, 0);
        lv_obj_align(lbl_inv_data_page, LV_ALIGN_CENTER, 0, 20);
    }
}
