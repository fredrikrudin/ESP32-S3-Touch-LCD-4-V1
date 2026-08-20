# Victron GUI v2 Display & Webserver for ESP32-S3

Ett fristående, open-source övervakningssystem byggt för **Waveshare ESP32-S3-Touch-LCD-4**. Projektet emulerar Victrons nya **GUI v2-gränssnitt** direkt på en 4-tums pekskärm samt publicerar en responsiv dashboard över ett lokalt webbgränssnitt. 

Systemet samlar in, avkodar och sammanställer strömdata från flera datakällor parallellt via Bluetooth (BLE).

## ✨ Funktioner

*   **Victron GUI v2 Estetik:** Modernt mörkt färgtema med runda kapslar och minimalistisk design på skärmen (via LVGL).
*   **Fixerad Statusrad:** Realtidsklocka synkroniserad via NTP (nätverkstid) samt Wi-Fi-indikator som visar aktuellt SSID och signalstyrka (RSSI i dBm).
*   **Stöd för flera enheter samtidigt:**
    *   Victron SmartShunt (Krypterad BLE)
    *   Victron MPPT Solcellsregulator (Krypterad BLE)
    *   Victron IP22 Batteriladdare (Krypterad BLE)
    *   Eco-Worthy LiFePO4 Batteri (Aktiv GATT/JBD-BMS anslutning)
*   **Enhetshantering & Knappar:** Varje datakälla kan individuellt aktiveras eller inaktiveras med On/Off-knappar både via pekskärmen och webbgränssnittet.
*   **Permanentminne (Preferences):** Alla MAC-adresser, krypteringsnycklar (bindkeys), Wi-Fi-uppgifter och inställningar sparas säkert i ESP32:ans interna flashminne.
*   **Fail-Safe Hotspot:** Om det sparade Wi-Fi-nätverket inte hittas, startar enheten automatiskt en egen accesspunkt (`VenusOS-ESP32-Setup`) så att du kan konfigurera enheten via mobilen.

## 🛠️ Maskinvara som krävs

1.  **Display:** [Waveshare ESP32-S3-Touch-LCD-4](https://waveshare.com) (480x480 pixlar, ST7701S RGB-skärm, GT911 kapacitiv touch).
2.  **Strömförsörjning:** 5V via USB-C eller via skärmens terminalblock (lämpligt för 12V-till-5V step-down i båt/husbil).

## 💻 Programvara & Bibliotek (Arduino IDE)

Före kompilering i Arduino IDE, se till att du använder ESP32-kärnan (v2.x eller v3.x) och installera följande bibliotek via Library Manager:

*   **LVGL (v8.x):** Light and Versatile Graphics Library för skärmgränssnittet.
*   **LovyanGFX:** Används som skärm- och touchdrivrutin (ST7701S / GT911) för att mata LVGL.
*   **ESPAsyncWebServer:** För den asynkrona webbservern.
*   **AsyncTCP:** Krävs av webbservern för ESP32.
*   *MbedTLS och Preferences är inbyggda i ESP32-kärnan och behöver inte installeras separat.*

## 🚀 Kom igång

1.  **Hämta dina Bindkeys:** Öppna **VictronConnect-appen** på din telefon, gå till din enhet -> Inställningar (kugghjulet) -> Produktinfo. Kopiera den 32 tecken långa hex-koden (`Krypteringsnyckel`).
2.  **Ladda upp koden:** Öppna `.ino`-filen i Arduino IDE, välj `ESP32S3 Dev Module` (eller Waveshares specifika kortprofil) och ladda upp koden.
3.  **Första konfiguration:** 
    *   Vid första start kommer skärmen visa "Frånkopplad".
    *   Anslut din mobil/dator till Wi-Fi-nätverket: `VenusOS-ESP32-Setup`.
    *   Öppna en webbläsare och gå till `http://192.168.4`.
    *   Skriv in ditt hemma-/båt-SSID, lösenord, samt dina enheters MAC-adresser och Bindkeys. Klicka på **Spara**.
4.  **Klart!** Enheten startar om, synkar klockan och börjar strömma data till din dashboard.

## 🔒 Säkerhetsnotering om Victron BLE
Victron använder **AES-128-CTR** kryptering för sin BLE-annonsering. Systemet använder ESP32-S3:ans hårdvaruacceleration för att dekryptera datapaketen i realtid utan att påverka skärmens flyt (60 FPS via LVGL).

## 📄 Licens
Detta projekt är licensierat under MIT-licensen - se [LICENSE](LICENSE) för mer information.
