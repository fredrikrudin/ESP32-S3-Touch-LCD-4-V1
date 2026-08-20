#ifndef VICTRON_DECRYPT_H
#define VICTRON_DECRYPT_H

#include <Arduino.h>
#include <mbedtls/aes.h>
#include "Config.h"

void hexStringToBytes(String hexStr, uint8_t* byteArr) {
    for (unsigned int i = 0; i < hexStr.length(); i += 2) {
        String byteString = hexStr.substring(i, i + 2);
        byteArr[i / 2] = strtol(byteString.c_str(), NULL, 16);
    }
}

bool decryptVictronPayload(uint8_t* manufacturerData, size_t dataLen, String bindkeyStr, uint8_t* decryptedOutput) {
    if (dataLen < 8) return false;
    uint8_t key[16];
    hexStringToBytes(bindkeyStr, key);

    uint8_t nonceLower = manufacturerData[3];
    uint8_t nonceUpper = manufacturerData[4];

    uint8_t iv[16];
    memset(iv, 0, 16);
    iv[0] = nonceLower;
    iv[1] = nonceUpper;

    size_t payloadLen = dataLen - 5; 
    uint8_t encryptedPayload[16];
    memcpy(encryptedPayload, &manufacturerData[5], payloadLen);

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 128);

    size_t nc_off = 0;
    uint8_t stream_block[16];
    memset(stream_block, 0, 16);

    int result = mbedtls_aes_crypt_ctr(&aes, payloadLen, &nc_off, iv, stream_block, encryptedPayload, decryptedOutput);
    mbedtls_aes_free(&aes);

    return (result == 0);
}

// --- NYTT: ÖVERSÄTT VICTRON LARM-BITMASK TILL TEXT ---
String parseInverterAlarms(uint16_t alarmMask) {
    if (alarmMask == 0) return "Inga larm (OK)";
    
    String alarmStr = "";
    if (alarmMask & (1 << 0)) alarmStr += "Låg batterispänning! ";
    if (alarmMask & (1 << 1)) alarmStr += "Överbelastning (Overload)! ";
    if (alarmMask & (1 << 2)) alarmStr += "Hög temperatur! ";
    if (alarmMask & (1 << 3)) alarmStr += "Hög DC-rippel! ";
    
    return alarmStr;
}

// --- UTÖKAD PARSNINGS-FUNKTION ---
void parseDecryptedData(uint8_t* decrypted, size_t len, int deviceType) {
    if (deviceType == 1) { // SmartShunt
        int16_t raw_volt = (decrypted[1] << 8) | decrypted[0];
        int32_t raw_curr = (decrypted[4] << 16) | (decrypted[3] << 8) | decrypted[2];
        shunt.voltage = raw_volt / 100.0;
        shunt.current = raw_curr / 1000.0;
        shunt.soc = decrypted[5] / 2.0;
    } 
    else if (deviceType == 2) { // MPPT
        int16_t raw_volt = (decrypted[1] << 8) | decrypted[0];
        int16_t raw_power = (decrypted[3] << 8) | decrypted[2];
        mppt.voltage = raw_volt / 100.0;
        mppt.current = (raw_volt > 0) ? (raw_power / mppt.voltage) : 0;
    }
    // --- NY ENHET: PHOENIX INVERTER (Type 3) ---
    else if (deviceType == 3) { 
        // Byte 0: State Code
        inverter.state_code = decrypted[0];
        
        // Byte 2-3: AC Power (Watt / VA)
        inverter.ac_watt = (decrypted[3] << 8) | decrypted[2];
        
        // Byte 4-5: Inverter Input Battery Voltage
        int16_t raw_inv_volt = (decrypted[5] << 8) | decrypted[4];
        inverter.battery_voltage = raw_inv_volt / 100.0;
        
        // Byte 6-7: Alarm/Warning Mask
        uint16_t raw_alarm = (decrypted[7] << 8) | decrypted[6];
        inverter.alarm_code = raw_alarm;

        Serial.printf("[Inverter] Status: %d, Effekt: %d W, Batteri: %.2fV\n", 
                      inverter.state_code, inverter.ac_watt, inverter.battery_voltage);
    }
}

#endif
