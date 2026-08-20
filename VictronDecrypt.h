#ifndef VICTRON_DECRYPT_H
#define VICTRON_DECRYPT_H

#include <Arduino.h>
#include <mbedtls/aes.h>

// Hjälpfunktion för att konvertera 32-karaktärers Hex-sträng (Bindkey) till 16 bytes array
void hexStringToBytes(String hexStr, uint8_t* byteArr) {
    for (unsigned int i = 0; i < hexStr.length(); i += 2) {
        String byteString = hexStr.substring(i, i + 2);
        byteArr[i / 2] = strtol(byteString.c_str(), NULL, 16);
    }
}

// Huvudfunktion för dekryptering
bool decryptVictronPayload(uint8_t* manufacturerData, size_t dataLen, String bindkeyStr, uint8_t* decryptedOutput) {
    if (dataLen < 8) return false; // För kort paket

    // 1. Förbered nyckeln (16 bytes)
    uint8_t key[16];
    hexStringToBytes(bindkeyStr, key);

    // 2. Extrahera Nonce/Data Counter (Byte 3 och 4 i Victrons tillverkardata)
    uint8_t nonceLower = manufacturerData[3];
    uint8_t nonceUpper = manufacturerData[4];

    // 3. Konstruera 16-bytes IV (Initialisation Vector) enligt Victrons specifikation
    uint8_t iv[16];
    memset(iv, 0, 16);
    iv[0] = nonceLower;
    iv[1] = nonceUpper;
    // Följande bytes (2-15) ska vara 0, utom sista blocken som mbedTLS använder som blockräknare (startar på 0)

    // 4. Separera den krypterade payloaden (Börjar på byte 5 i tillverkardata)
    size_t payloadLen = dataLen - 5; 
    uint8_t encryptedPayload[16]; // Victrons paket är sällan över 16 bytes
    memcpy(encryptedPayload, &manufacturerData[5], payloadLen);

    // 5. Utför AES-128-CTR dekryptering via mbedTLS
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 128); // Sätt 128-bitars krypteringsnyckel

    size_t nc_off = 0;
    uint8_t stream_block[16];
    memset(stream_block, 0, 16);

    // MbedTLS AES-CTR funktion dekrypterar datan direkt på plats (in-place eller till output)
    int result = mbedtls_aes_crypt_ctr(&aes, payloadLen, &nc_off, iv, stream_block, encryptedPayload, decryptedOutput);

    mbedtls_aes_free(&aes);

    return (result == 0); // Returnerar sant om dekrypteringen lyckades utan fel
}

#endif
