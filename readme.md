# Victron GUI v2 Display, Webserver & Automation Controller for ESP32-S3

Ett kraftfullt och helt fristående övervaknings- och automationssystem byggt för **Waveshare ESP32-S3-Touch-LCD-4**. Projektet emulerar Victrons moderna **GUI v2-gränssnitt** direkt på den 4-tums stora pekskärmen, hanterar extern I2C-maskinvara, erbjuder avancerad tidsstyrning samt skyddar dina data med en säker inloggningssida över det lokala webbgränssnittet.

Systemet samlar in och bearbetar energidata från flera källor parallellt via Bluetooth (BLE), utvärderar tidtabeller, styr fysiska reläer samt sparar energi genom adaptiv radiopollning.

## ✨ Nyckelfunktioner

*   **Victron GUI v2 Estetik:** Minimalistisk design med runda kapslar och mörkt färgtema baserat på Victrons senaste användargränssnitt.
*   **Fixerad Statusrad:** Realtidsklocka synkroniserad via NTP (nätverkstid) samt Wi-Fi-indikator som visar live SSID och signalstyrka (RSSI i dBm).
*   **Stöd för 5 parallella enheter (Modulär arkitektur):**
    *   Victron SmartShunt (Krypterad BLE via AES-128-CTR)
    *   Victron MPPT Solcellsregulator (Krypterad BLE via AES-128-CTR)
    *   Victron IP22 Batteriladdare (Krypterad BLE via AES-128-CTR)
    *   **Victron Phoenix Pure Sine Inverter:** Helt integrerad i systemet med en dedikerad statussida (LVGL/Webb) som *endast* visas när enheten är markerad som aktiv.
    *   Eco-Worthy LiFePO4 Batteri (Aktiv GATT/BLE-anslutning mot JBD-BMS)
*   **4-Kanals I2C Reläkontroll:** Styrning av en **ELEGOO 4-kanals DC 5V relämodul** över I2C-bussen med hjälp av en **PCF8574 I/O-expander** för att spara värdefulla GPIO-pinnar på ESP32:an.
*   **Avancerad Schemaläggning (Timer):** Möjlighet att sätta individuella start- och stopptidpunkter (timmar och minuter) för varje relä via en dedikerad schemasida på webben.
*   **Intelligent PWM-Dimning (Strömspar & Nattläge):**
    *   **Auto-Dim:** Skärmen dimmas automatiskt ner till 10 % ljusstyrka efter 30 sekunders inaktivitet.
    *   **Touch-to-Wake:** Ett tryck var som helst på pekskärmen tänder omedelbart upp belysningen till full styrka.
    *   **Tidsstyrt Nattläge:** Mellan kl. 22:00 och 06:00 sänks den maximala ljusstyrkan till behagliga 30 % för att inte blända i mörka miljöer.
*   **🔋 Adaptiv BLE-Strömsparning (Pollning):** Möjlighet att ställa in dataintervall (1s, 10s, 30s, eller 60s) via webbgränssnittet. Mellan mätningarna stängs ESP32-S3:ans BLE-radio av helt vilket drastiskt minskar strömförbrukningen och värmeutvecklingen i displayen.
*   **🔒 Session-baserad Webbsäkerhet:** Alla skyddade sidor och JSON-datapaket blockeras av en cookie-baserad inloggningsskärm (`HttpOnly`). Användarnamn är fast till `admin` och lösenordet hanteras i permanentminnet.
*   **Permanentminne (Preferences):** Alla MAC-adresser, krypteringsnycklar (bindkeys), nätverksuppgifter, reläscheman, pollningsintervall och administrationslösenord sparas i ESP32:ans interna flashminne.

## 🔌 Hårdvarukonfiguration & Inkoppling

För att ansluta ELEGOO-reläkortet över I2C används en PCF8574-modul. Eftersom Elegoo använder **Active-LOW**-reläer drar reläet när pinnen blir låg (0V).

### Kopplingsschema:
1.  **SDA (I2C Data):** Anslut från Waveshare Display till pinne **SDA** på PCF8574 (I koden mappad till GPIO 19).
2.  **SCL (I2C Clock):** Anslut från Waveshare Display till pinne **SCL** på PCF8574 (I koden mappad till GPIO 20).
3.  **PCF8574 till Reläkort:** 
    *   Utgång **P0** -> `IN1` (Relä 1)
    *   Utgång **P1** -> `IN2` (Relä 2)
    *   Utgång **P2** -> `IN3` (Relä 3)
    *   Utgång **P3** -> `IN4` (Relä 4)
4.  **Strömförsörjning:** Ge både PCF8574 och Elegoo-reläkortet **5V** och gemensam **GND** från ditt system.

---

## 💻 Kompileringsguide för Arduino IDE

Följ dessa steg noggrant för att installera rätt verktyg och kompilera projektet utan fel.

### Steg 1: Förbered Projektmappen
1. Skapa en mapp på din dator med namnet `VenusOS-ESP32-Display`.
2. Spara följande fem filer i denna mapp:
   * `VenusOS-ESP32-Display.ino`
   * `Config.h`
   * `VictronDecrypt.h`
   * `Webpages_Core.h`
   * `Webpages_Config.h`

### Steg 2: Installera ESP32-kortstöd i Arduino IDE
1. Öppna Arduino IDE.
2. Gå till **File** -> **Preferences** (eller **Arduino IDE** -> **Settings** på Mac).
3. I fältet *Additional Boards Manager URLs*, klistra in följande länk:
   `https://githubusercontent.com`
4. Klicka på **OK**.
5. Gå till **Tools** -> **Board** -> **Boards Manager...**
6. Sök efter `esp32` (av Espressif Systems) och klicka på **Install** (version 2.x eller 3.x rekommenderas).

### Steg 3: Installera Bibliotek som Krävs
Gå till **Tools** -> **Manage Libraries...** och sök efter samt installera följande bibliotek:
1. **LVGL** (Sök efter `lvgl` av *Light and Versatile Graphics Library* – välj version 8.x då källkoden är skriven för v8 API).
2. **LovyanGFX** (Sök efter `LovyanGFX` – används för Waveshare-skärmens ST7701S RGB- och GT911 touch-drivrutiner).
3. **ESPAsyncWebServer** (Installera den officiella eller社区/community-versionen för asynkron webbserver på ESP32).
4. **AsyncTCP** (Krävs som nätverkskomponent till webbservern).

### Steg 4: Konfigurera LVGL
När LVGL har installerats måste du tillhandahålla en konfigurationsfil:
1. Gå till din dators dokumentmapp där Arduino-biblioteken sparas (oftast `Documents/Arduino/libraries/lvgl/`).
2. Kopiera filen `lv_conf_template.h` och klistra in den i mappen direkt *ovanför* (`Documents/Arduino/libraries/`).
3. Döp om den kopierade filen till exakt `lv_conf_template.h` -> `lv_conf.h`.
4. Öppna `lv_conf.h` i en texteditor, leta upp rad 15 (`#if 0`) och ändra den till `#if 1` för att aktivera filen.

### Steg 5: Välj Kortinställningar i Arduino IDE
Gå till mappen `VenusOS-ESP32-Display` och öppna `VenusOS-ESP32-Display.ino`. Alla fem filer kommer öppnas som separata flikar. Ställ in följande under **Tools**-menyn:
*   **Board:** `ESP32S3 Dev Module` (eller den specifika Waveshare ESP32-S3-Touch kortprofilen om du har installerat den).
*   **USB CDC On Boot:** `Enabled` (Viktigt för att kunna se `Serial.print` utskrifter i Serial Monitor via USB).
*   **Flash Size:** `16MB (128Mb)` (Anpassa efter din Waveshare-hårdvara, oftast 8MB eller 16MB).
*   **Partition Scheme:** `16M Flash (3MB APP/9.9MB FATFS)` eller `Huge APP (3MB No OTA/1MB SPIFFS)` för att rymma både kod och det grafiska biblioteket.
*   **Port:** Välj den COM-port som din Waveshare-skärm är ansluten till.

### Steg 6: Verifiera och Ladda upp
1. Klicka på **Verify** (bock-ikonen) högst upp till vänster för att kontrollera att allt kompilerar utan fel.
2. Klicka på **Upload** (pil-ikonen) för att skriva programmet till din ESP32-S3.

---

## 🚀 Första Start & Konfiguration

1.  **Fail-Safe Hotspot:** Om enheten inte lyckas ansluta till ditt Wi-Fi vid första uppstarten, skapar den en egen accesspunkt med namnet: `VenusOS-ESP32-Setup`.
2.  **Öppna Panel:** Anslut till nätverket med din mobil/dator och surfa till `http://192.168.4`.
3.  **Logga in:** Standarduppgifter vid första start:
    *   **Användarnamn:** `admin`
    *   **Lösenord:** `admin123`
4.  **Konfigurera Hårdvara:**
    *   Skriv in ditt båt-/husbils-Wi-Fi under nätverksinställningar.
    *   **Ändra administrationslösenordet** under *Webbsäkerhet* för att låsa nätverket.
    *   Aktivera dina önskade Victron-enheter (inklusive Phoenix Inverter och Eco-Worthy) och klistra in deras respektive MAC-adresser och 32-karaktärers `bindkey` (nyckeln hämtas via VictronConnect-appen).
    *   Välj önskad **Pollning/Dataintervall** (t.ex. 30 sekunder för maximal strömsparning).
5.  Klicka på **Spara & Starta om**. Enheten sparar parametrarna till permanentminnet, synkroniserar klockan mot en svensk NTP-server (`pool.ntp.org`) och startar upp i säkert driftläge.

## 📄 Licens
Detta projekt är open-source och licensierat under MIT-licensen - se [LICENSE](LICENSE) för detaljer.

##AI:
https://share.google/aimode/CUZ8ZNv4NKfIRGgOR




