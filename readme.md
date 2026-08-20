# Victron GUI v2 Display, Webserver & Automation Controller for ESP32-S3

Ett kraftfullt och helt fristående övervaknings- och automationssystem byggt för **Waveshare ESP32-S3-Touch-LCD-4**. Projektet emulerar Victrons moderna **GUI v2-gränssnitt** direkt på den 4-tums stora pekskärmen, hanterar extern I2C-maskinvara, erbjuder avancerad tidsstyrning samt skyddar dina data med en säker inloggningssida över det lokala webbgränssnittet.

Systemet samlar in och bearbetar energidata från flera källor parallellt via Bluetooth (BLE), utvärderar tidtabeller och styr fysiska reläer.

## ✨ Nyckelfunktioner

*   **Victron GUI v2 Estetik:** Minimalistisk design med runda kapslar och mörkt färgtema baserat på Victrons senaste användargränssnitt.
*   **Fixerad Statusrad:** Realtidsklocka synkroniserad via NTP (nätverkstid) samt Wi-Fi-indikator som visar live SSID och signalstyrka (RSSI i dBm).
*   **Stöd för 5 parallella enheter (Modulär arkitektur):**
    *   Victron SmartShunt (Krypterad BLE via AES-128-CTR)
    *   Victron MPPT Solcellsregulator (Krypterad BLE via AES-128-CTR)
    *   Victron IP22 Batteriladdare (Krypterad BLE via AES-128-CTR)
    *   **Victron Phoenix Pure Sine Inverter:** Helt integrerad i systemet med en dedikerad statussida (LVGL/Webb) som *endast* visas när enheten är markerad som aktiv.
    *   Eco-Worthy LiFePO4 Batteri (Aktiv GATT-anslutning mot JBD-BMS)
*   **4-Kanals I2C Reläkontroll:** Styrning av en **ELEGOO 4-kanals DC 5V relämodul** över I2C-bussen med hjälp av en **PCF8574 I/O-expander** för att spara värdefulla GPIO-pinnar på ESP32:an.
*   **Avancerad Schemaläggning (Timer):** Möjlighet att sätta individuella start- och stopptidpunkter (timmar och minuter) för varje relä via en dedikerad schemasida på webben.
*   **Intelligent PWM-Dimning (Strömspar & Nattläge):**
    *   **Auto-Dim:** Skärmen dimmas automatiskt ner till 10 % ljusstyrka efter 30 sekunders inaktivitet.
    *   **Touch-to-Wake:** Ett tryck var som helst på pekskärmen tänder omedelbart upp belysningen till full styrka.
    *   **Tidsstyrt Nattläge:** Mellan kl. 22:00 och 06:00 sänks den maximala ljusstyrkan till behagliga 30 % för att inte blända i mörka miljöer.
*   **🔒 Session-baserad Webbsäkerhet:** Alla skyddade sidor och JSON-datapaket blockeras av en cookie-baserad inloggningsskärm (`HttpOnly`). Användarnamn är fast till `admin` och lösenordet hanteras i permanentminnet.
*   **Permanentminne (Preferences):** Alla MAC-adresser, krypteringsnycklar (bindkeys), nätverksuppgifter, reläscheman och administrationslösenord sparas i ESP32:ans interna flashminne.

## 🔌 Hårdvarukonfiguration & Inkoppling

För att ansluta ELEGOO-reläkortet över I2C används en PCF8574-modul. Eftersom Elegoo använder **Active-LOW**-reläer drar reläet när pinnen blir låg (0V).

### Kopplingsschema:
1.  **SDA (I2C Data):** Anslut från Waveshare Display till pinne **P0** (eller motsvarande) på PCF8574. (I koden mappad till GPIO 19).
2.  **SCL (I2C Clock):** Anslut från Waveshare Display till pinne **P1** (eller motsvarande) på PCF8574. (I koden mappad till GPIO 20).
3.  **PCF8574 till Reläkort:** 
    *   Utgång **P0** -> `IN1` (Relä 1)
    *   Utgång **P1** -> `IN2` (Relä 2)
    *   Utgång **P2** -> `IN3` (Relä 3)
    *   Utgång **P3** -> `IN4` (Relä 4)
4.  **Strömförsörjning:** Ge både PCF8574 och Elegoo-reläkortet **5V** och gemensam **GND** från ditt system.

## 💻 Programvara & Bibliotek (Arduino IDE)

Se till att ESP32-kärnan är installerad och hämta följande bibliotek via Library Manager innan du laddar upp koden:

*   **LVGL (v8.x):** Grafikmotor för pekskärmens gränssnittet.
*   **LovyanGFX:** Skärm- och touchdrivrutin (ST7701S / GT911) konfigurerad för att mata LVGL.
*   **ESPAsyncWebServer:** För den säkra, asynkrona webbservern.
*   **AsyncTCP:** Nätverksstöd för webbservern.
*   *Wire, Preferences, Time och MbedTLS är inbyggda i ESP32-kärnan.*

## 🚀 Kom igång

1.  **Ladda upp koden:** Skapa en mapp som heter `VenusOS-ESP32-Display` på din dator. Spara projektets tre filer (`VenusOS-ESP32-Display.ino`, `Config.h`, `Webpages.h`) i mappen och flasha till din ESP32-S3.
2.  **Första start (Fail-Safe Hotspot):** 
    *   Om inget Wi-Fi hittas startar enheten en egen accesspunkt: `VenusOS-ESP32-Setup`.
    *   Anslut med din mobil/dator och surfa till `http://192.168.4`.
3.  **Logga in på Webben:**
    *   Första gången du besöker gränssnittet möts du av en inloggningsskärm.
    *   Standarduppgifter: Användarnamn: `admin` | Lösenord: `admin123`.
4.  **Konfigurera systemet:**
    *   Ange ditt hemma-/båt-Wi-Fi under nätverk.
    *   **Ändra administrationslösenordet** under sektionen *Webbsäkerhet* för att låsa nätverket.
    *   Aktivera dina önskade Victron-enheter (inklusive Phoenix Inverter om du har en) och klistra in deras respektive MAC-adresser och 32-karaktärers `bindkey` (hämtas via VictronConnect-appen).
    *   Klicka på **Spara och starta om**.
5.  **Drift:** Enheten startar nu upp i säkert läge, synkroniserar nätverkstiden, och börjar köra reläscheman, auto-dimning samt energivisning.

## 📅 Användning av Schemaläggaren
Gå till `/scheduler` via webbgränssnittet (kräver inloggning). Här kan du slå på timers för valfria reläer och ställa in exakta tidsfönster (t.ex. starta en vattenpump kl. 08:00 och stänga av den kl. 08:30). Systemet utvärderar tidtabellerna en gång var 30:e sekund mot den interna NTP-klockan.

## 📄 Licens
Detta projekt är open-source och licensierat under MIT-licensen - se [LICENSE](LICENSE) för detaljer.
