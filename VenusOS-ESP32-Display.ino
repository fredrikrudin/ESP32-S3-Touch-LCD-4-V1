#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <time.h>
#include <Wire.h>
#include <lvgl.h>
#include "Config.h"
#include "Webpages.h"

Preferences prefs; AsyncWebServer server(80);
VictronDevice shunt; VictronDevice mppt; VictronDevice ip22; EcoWorthyDevice ecoBatt; PhoenixInverter inverter;

String wifi_ssid, wifi_pass; int update_interval;
String web_password; // Sparat administrationslösenord
byte relay_states = 0x00; RelaySchedule relay_sched;
unsigned long last_touch_time = 0; int current_brightness = 100;

lv_obj_t * lbl_clock; lv_obj_t * lbl_wifi;

// --- KONTROLLERA OM ANVÄNDAREN ÄR INLOGGAD via COOKIE ---
bool is_authenticated(AsyncWebServerRequest *request) {
    if (request->hasHeader("Cookie")) {
        AsyncWebHeader* cookie = request->getHeader("Cookie");
        // Kontrollera om vår unika token finns i cookien
        if (cookie->value().indexOf("venus_session=authenticated") != -1) {
            return true;
        }
    }
    return false;
}

String advancedProcessor(const String& var) {
    if(var == "SSID") return wifi_ssid;
    if(var == "PASS") return wifi_pass;
    if(var == "WEBPASS") return web_password; // Processor-länk
    if(var == "SH_MAC") return shunt.mac;
    return String();
}

void loadSettings() {
    prefs.begin("v_mod", false);
    wifi_ssid = prefs.getString("ssid", "DITT_SSID");
    wifi_pass = prefs.getString("pass", "DITT_LÖSENORD");
    web_password = prefs.getString("web_pass", "admin123"); // Standardlösenord om inget satts
    update_interval = prefs.getInt("interval", 2);
    shunt.enabled = prefs.getBool("sh_en", false);
    shunt.mac = prefs.getString("sh_mac", "00:00:00:00:00:00");
    prefs.end();
}

void setup() {
    Serial.begin(115200); Wire.begin(19, 20); loadSettings();
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

    // --- 1. ENDPOINT: INLOGGNINGSSIDA (ÖPPEN FÖR ALLA) ---
    server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_200(login_html, "text/html");
    });

    // --- 2. ENDPOINT: UT-VÄRDERA INLOGGNINGS-POST ---
    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
        String user = "";
        String pass = "";
        
        if(request->hasParam("username", true)) user = request->getParam("username", true)->value();
        if(request->hasParam("password", true)) pass = request->getParam("password", true)->value();
        
        if(user == "admin" && pass == web_password) {
            AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Ok");
            // Sätt en session-cookie till webbläsaren vid rätt lösenord
            response->addHeader("Set-Cookie", "venus_session=authenticated; Path=/; HttpOnly");
            response->addHeader("Location", "/");
            request->send(response);
        } else {
            // Fel lösenord -> skicka tillbaka till login
            request->redirect("/login");
        }
    });

    // --- 3. SKYDDADE ENDPOINTS (KRÄVER INLOGGNING) ---
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
        if(!is_authenticated(request)) return request->redirect("/login");
        request->send_200(index_html, "text/html"); 
    });
    
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){ 
        if(!is_authenticated(request)) return request->redirect("/login");
        request->send_200(settings_html, "text/html", advancedProcessor); 
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!is_authenticated(request)) return request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
        time_t now; struct tm ti; char t_buf = "--:--"; if(getLocalTime(&ti)) sprintf(t_buf, "%02d:%02d", ti.tm_hour, ti.tm_min);
        String json = "{\"clock\":\"" + String(t_buf) + "\",\"wifi_ssid\":\"" + WiFi.SSID() + "\",\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
        json += "\"shunt_v\":" + String(shunt.voltage) + ",\"shunt_a\":" + String(shunt.current) + "}";
        request->send(200, "application/json", json);
    });

    server.on("/save_advanced", HTTP_POST, [](AsyncWebServerRequest *request){
        if(!is_authenticated(request)) return request->send(401, "text/plain", "Unauthorized");
        prefs.begin("v_mod", false);
        if(request->hasParam("ssid", true)) prefs.putString("ssid", request->getParam("ssid", true)->value());
        if(request->hasParam("pass", true)) prefs.putString("pass", request->getParam("pass", true)->value());
        
        // Spara det nya webblösenordet om det angivits i fältet
        if(request->hasParam("web_pass", true) && request->getParam("web_pass", true)->value().length() >= 4) {
            prefs.putString("web_pass", request->getParam("web_pass", true)->value());
        }

        if(request->hasParam("sh_mac", true)) prefs.putString("sh_mac", request->getParam("sh_mac", true)->value());
        prefs.end();
        request->send(200, "text/html", "<h3>Sparat! Startar om...</h3>");
        delay(1500); ESP.restart();
    });

    server.begin();
    // [init_backlight och init_lvgl_interface körs som vanligt här under]
}

void loop() {
    lv_timer_handler();
    delay(5);
}
