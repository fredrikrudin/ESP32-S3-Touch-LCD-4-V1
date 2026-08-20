#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <time.h>
#include <Wire.h>
#include <lvgl.h>
#include "Config.h"
#include "Webpages_Core.h"
#include "Webpages_Config.h"

Preferences prefs; 
AsyncWebServer server(80);

VictronDevice shunt; VictronDevice mppt; VictronDevice ip22; EcoWorthyDevice ecoBatt; PhoenixInverter inverter;
String wifi_ssid, wifi_pass, web_password; 
int update_interval;
byte relay_states = 0x00; 
RelaySchedule relay_sched[4];
unsigned long last_touch_time = 0; int current_brightness = 100;

lv_obj_t * lbl_clock; lv_obj_t * lbl_wifi;
lv_obj_t * cb_shunt; lv_obj_t * cb_mppt; lv_obj_t * cb_ip22; lv_obj_t * cb_eco; lv_obj_t * cb_inv;
lv_obj_t * btn_relays[4];
lv_obj_t * tabview; lv_obj_t * tab_inverter;
lv_obj_t * lbl_inv_data_page = NULL;
BLEScan* pBLEScan;

// --- KORRIGERING: Implementation av Wi-Fi Symbol ---
const char* get_wifi_symbol(int rssi) {
    if (rssi >= -50) return "📶 [Utmärkt]";
    if (rssi >= -70) return "📶 [Bra]";
    if (rssi >= -85) return "📶 [Svag]";
    return "❌ [Ingen signal]";
}

bool is_authenticated(AsyncWebServerRequest *request) {
    if (request->hasHeader("Cookie")) {
        AsyncWebHeader* cookie = request->getHeader("Cookie");
        if (cookie->value().indexOf("venus_session=authenticated") != -1) return true;
    }
    return false;
}

String advancedProcessor(const String& var) {
    if(var == "SSID") return wifi_ssid;
    if(var == "PASS") return wifi_pass;
    if(var == "WEBPASS") return web_password;
    if(var == "INV_EN") return inverter.enabled ? "checked" : "";
    if(var == "INV_MAC") return inverter.mac;
    if(var == "INV_KEY") return inverter.key;
    if(var == "SH_EN") return shunt.enabled ? "checked" : "";
    if(var == "SH_MAC") return shunt.mac;
    if(var == "SH_KEY") return shunt.key;
    if(var == "INT1") return (update_interval == 1) ? "selected" : "";
    if(var == "INT10") return (update_interval == 10) ? "selected" : "";
    if(var == "INT30") return (update_interval == 30) ? "selected" : "";
    if(var == "INT60") return (update_interval == 60) ? "selected" : "";
    if (var == "SCHED_CARDS") {
        String html = "";
        for(int i=0; i<4; i++) {
            html += "<div class='card'><h3>📅 Relä " + String(i+1) + "</h3>";
            html += "<div class='row'><label>Aktivera:</label><input type='checkbox' name='en_" + String(i) + "' value='1' " + (relay_sched[i].enabled ? "checked" : "") + "></div>";
            html += "<div class='row'><label>Start (HH:MM):</label><div><input type='number' name='sh_" + String(i) + "' min='0' max='23' value='" + String(relay_sched[i].start_hour) + "'> : <input type='number' name='sm_" + String(i) + "' min='0' max='59' value='" + String(relay_sched[i].start_min) + "'></div></div>";
            html += "<div class='row'><label>Stopp (HH:MM):</label><div><input type='number' name='eh_" + String(i) + "' min='0' max='23' value='" + String(relay_sched[i].end_hour) + "'> : <input type='number' name='em_" + String(i) + "' min='0' max='59' value='" + String(relay_sched[i].end_min) + "'></div></div></div>";
        }
        return html;
    }
    return String();
}

void loadSettings() {
    prefs.begin("v_mod", true);
    wifi_ssid = prefs.getString("ssid", "DITT_SSID");
    wifi_pass = prefs.getString("pass", "DITT_LÖSENORD");
    web_password = prefs.getString("web_pass", "admin123");
    update_interval = prefs.getInt("interval", 2);
    inverter.enabled = prefs.getBool("inv_en", false);
    inverter.mac = prefs.getString("inv_mac", "00:00:00:00:00:00");
    inverter.key = prefs.getString("inv_key", "");
    shunt.enabled = prefs.getBool("sh_en", false);
    shunt.mac = prefs.getString("sh_mac", "00:00:00:00:00:00");
    shunt.key = prefs.getString("sh_key", "");
    for(int i=0; i<4; i++) {
        relay_sched[i].enabled = prefs.getBool(("re_" + String(i)).c_str(), false);
        relay_sched[i].start_hour = prefs.getInt(("rsh_" + String(i)).c_str(), 0);
        relay_sched[i].start_min = prefs.getInt(("rsm_" + String(i)).c_str(), 0);
        relay_sched[i].end_hour = prefs.getInt(("reh_" + String(i)).c_str(), 0);
        relay_sched[i].end_min = prefs.getInt(("rem_" + String(i)).c_str(), 0);
    }
    prefs.end();
}

void write_relays(byte state_mask) {
    byte pcf_data = 0xF0 | (~state_mask & 0x0F);
    Wire.beginTransmission(PCF8574_I2C_ADDRESS); Wire.write(pcf_data); Wire.endTransmission();
}

void check_relay_schedules() {
    struct tm ti; if (!getLocalTime(&ti)) return;
    int current_minutes = (ti.tm_hour * 60) + ti.tm_min;
    byte new_states = relay_states;
    for (int i = 0; i < 4; i++) {
        if (!relay_sched[i].enabled) continue;
        int start_total = (relay_sched[i].start_hour * 60) + relay_sched[i].start_min;
        int end_total = (relay_sched[i].end_hour * 60) + relay_sched[i].end_min;
        bool inside = (start_total < end_total) ? (current_minutes >= start_total && current_minutes < end_total) : (current_minutes >= start_total || current_minutes < end_total);
        if (inside) new_states |= (1 << i); else new_states &= ~(1 << i);
    }
    if (new_states != relay_states) { relay_states = new_states; write_relays(relay_states); }
}
void init_backlight() { ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES); ledcAttachPin(BACKLIGHT_PIN, PWM_CHANNEL); set_backlight(100); last_touch_time = millis(); }
void set_backlight(int percentage) { if (percentage < 0) percentage = 0; if (percentage > 100) percentage = 100; current_brightness = percentage; ledcWrite(PWM_CHANNEL, (percentage * 255) / 100); }
void update_display_dimming() { struct tm ti; bool is_night = (getLocalTime(&ti) && (ti.tm_hour >= 22 || ti.tm_hour < 6)); if (millis() - last_touch_time > 30000) set_backlight(10); else set_backlight(is_night ? 30 : 100); }
static void global_touch_feedback_cb(lv_event_t * e) { last_touch_time = millis(); update_display_dimming(); }

static void checkbox_event_cb(lv_event_t * e) {
    lv_obj_t * cb = lv_event_get_target(e); bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
    prefs.begin("v_mod", false); if(cb == cb_inv) { prefs.putBool("inv_en", checked); inverter.enabled = checked; } prefs.end();
}
static void relay_click_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e); int id = (int)lv_event_get_user_data(e);
    relay_states ^= (1 << id); write_relays(relay_states);
    lv_obj_set_style_bg_color(btn, (relay_states & (1 << id)) ? lv_color_hex(0x4CAF50) : lv_color_hex(0x282C34), 0);
}
static void status_bar_update_cb(lv_timer_t * timer) {
    struct tm ti; if (getLocalTime(&ti)) lv_label_set_text_fmt(lbl_clock, "%02d:%02d", ti.tm_hour, ti.tm_min);
    if (WiFi.status() == WL_CONNECTED) lv_label_set_text_fmt(lbl_wifi, "%s  %s (%d dBm)", get_wifi_symbol(WiFi.RSSI()), WiFi.SSID().c_str(), WiFi.RSSI());
    else lv_label_set_text(lbl_wifi, "❌ Frånkopplad");
}

void init_lvgl_interface() {
    lv_obj_add_event_cb(lv_scr_act(), global_touch_feedback_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_t * status_bar = lv_obj_create(lv_scr_act()); lv_obj_set_size(status_bar, 480, 35); lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0); lv_obj_set_style_border_width(status_bar, 0, 0); lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    lbl_clock = lv_label_create(status_bar); lv_obj_align(lbl_clock, LV_ALIGN_LEFT_MID, 10, 0);
    lbl_wifi = lv_label_create(status_bar); lv_obj_align(lbl_wifi, LV_ALIGN_CENTER, 0, 0);
    lv_timer_create(status_bar_update_cb, 1000, NULL);

    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 40); lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, 0); lv_obj_set_size(tabview, 480, 445);
    lv_obj_set_style_bg_color(tabview, lv_color_hex(0x0B0C0E), 0);
    lv_obj_t * tab_dash = lv_tabview_add_tab(tabview, "Dashboard");

    if (inverter.enabled) {
        tab_inverter = lv_tabview_add_tab(tabview, "Inverter"); lv_obj_set_style_bg_color(tab_inverter, lv_color_hex(0x0B0C0E), 0);
        lv_obj_t * cap_inv = lv_obj_create(tab_inverter); lv_obj_set_size(cap_inv, 420, 150); lv_obj_align(cap_inv, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(cap_inv, 25, 0); lv_obj_set_style_bg_color(cap_inv, lv_color_hex(0x181A1F), 0); lv_obj_set_style_border_color(cap_inv, lv_color_hex(0xE67E22), 0);
        lbl_inv_data_page = lv_label_create(cap_inv); lv_label_set_text(lbl_inv_data_page, "Väntar på BLE-data..."); 
        lv_obj_align(lbl_inv_data_page, LV_ALIGN_CENTER, 0, 10);
    }

    lv_obj_t * tab_hw = lv_tabview_add_tab(tabview, "Hårdvara");
    lv_obj_set_flex_flow(tab_hw, LV_FLEX_FLOW_COLUMN);
    
    cb_inv = lv_checkbox_create(tab_hw); lv_checkbox_set_text(cb_inv, "Aktivera Phoenix Inverter");
    if(inverter.enabled) lv_obj_add_state(cb_inv, LV_STATE_CHECKED);
    lv_obj_add_event_cb(cb_inv, checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    for(int i=0; i<4; i++) {
        btn_relays[i] = lv_btn_create(tab_dash); lv_obj_set_size(btn_relays[i], 120, 50);
        lv_obj_align(btn_relays[i], LV_ALIGN_CENTER, (i % 2 == 0 ? -70 : 70), (i < 2 ? 60 : 120));
        lv_obj_set_style_bg_color(btn_relays[i], lv_color_hex(0x282C34), 0);
        lv_obj_t * lbl = lv_label_create(btn_relays[i]); lv_label_set_text_fmt(lbl, "Relä %d", i+1); lv_obj_center(lbl);
        lv_obj_add_event_cb(btn_relays[i], relay_click_cb, LV_EVENT_CLICKED, (void*)i);
    }
}

void setup() {
    Serial.begin(115200); Wire.begin(19, 20); write_relays(0x00); init_backlight(); loadSettings();
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    int timeout = 0; while (WiFi.status() != WL_CONNECTED && timeout < 12) { delay(500); timeout++; }
    if (WiFi.status() == WL_CONNECTED) configTime(3600, 3600, "0.se.pool.ntp.org", "1.se.pool.ntp.org");
    else WiFi.softAP("VenusOS-ESP32-Setup");

    // --- KORRIGERING: send_200 ersatt med send_P ---
    server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(200, "text/html", login_html); });
    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
        String user = request->hasParam("username", true) ? request->getParam("username", true)->value() : "";
        String pass = request->hasParam("password", true) ? request->getParam("password", true)->value() : "";
        if(user == "admin" && pass == web_password) {
            AsyncWebServerResponse *res = request->beginResponse(302, "text/plain", "Ok");
            res->addHeader("Set-Cookie", "venus_session=authenticated; Path=/; HttpOnly");
            res->addHeader("Location", "/"); request->send(res);
        } else { request->redirect("/login"); }
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ if(!is_authenticated(request)) return request->redirect("/login"); request->send_P(200, "text/html", index_html); });
    
    // --- KORRIGERING: Korrekt anrop för mall-processorn (advancedProcessor) ---
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){ if(!is_authenticated(request)) return request->redirect("/login"); request->send(200, "text/html", settings_html, advancedProcessor); });
    server.on("/scheduler", HTTP_GET, [](AsyncWebServerRequest *request){ if(!is_authenticated(request)) return request->redirect("/login"); request->send(200, "text/html", scheduler_html, advancedProcessor); });
    
    server.on("/toggle_relay", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("id")) { int id = request->getParam("id")->value().toInt(); relay_states ^= (1 << id); write_relays(relay_states); }
        request->send(200, "text/plain", "OK");
    });

    server.on("/save_schedule", HTTP_POST, [](AsyncWebServerRequest *request){
        if(!is_authenticated(request)) return request->send(401, "text/plain", "Unauthorized");
        prefs.begin("v_mod", false);
        for(int i=0; i<4; i++) {
            relay_sched[i].enabled = request->hasParam("en_" + String(i), true);
            if(request->hasParam("sh_" + String(i), true)) relay_sched[i].start_hour = request->getParam("sh_" + String(i), true)->value().toInt();
            if(request->hasParam("sm_" + String(i), true)) relay_sched[i].start_min = request->getParam("sm_" + String(i), true)->value().toInt();
            if(request->hasParam("eh_" + String(i), true)) relay_sched[i].end_hour = request->getParam("eh_" + String(i), true)->value().toInt();
            if(request->hasParam("em_" + String(i), true)) relay_sched[i].end_min = request->getParam("em_" + String(i), true)->value().toInt();
            prefs.putBool(("re_" + String(i)).c_str(), relay_sched[i].enabled);
            prefs.putInt(("rsh_" + String(i)).c_str(), relay_sched[i].start_hour); prefs.putInt(("rsm_" + String(i)).c_str(), relay_sched[i].start_min);
            prefs.putInt(("reh_" + String(i)).c_str(), relay_sched[i].end_hour); prefs.putInt(("rem_" + String(i)).c_str(), relay_sched[i].end_min);
        }
        prefs.end(); request->redirect("/");
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!is_authenticated(request)) return request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
        struct tm ti; 
        char t_buf[6] = "--:--"; // KORRIGERING: char till char-array med dubbeluttfnuttar
        if(getLocalTime(&ti)) sprintf(t_buf, "%02d:%02d", ti.tm_hour, ti.tm_min);
        String json = "{\"clock\":\"" + String(t_buf) + "\",\"wifi_ssid\":\"" + WiFi.SSID() + "\",\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
        json += "\"relays\":" + String(relay_states) + ",\"inv_active\":" + String(inverter.enabled ? "true" : "false") + ",";
        json += "\"inv_w\":" + String(inverter.ac_watt) + ",\"inv_state\":\"" + String(get_inverter_state_str(inverter.state_code)) + "\",";
        json += "\"inv_alarm\":\"\",\"shunt_v\":" + String(shunt.voltage) + ",\"shunt_a\":" + String(shunt.current) + "}";
        request->send(200, "application/json", json);
    });

    server.on("/save_advanced", HTTP_POST, [](AsyncWebServerRequest *request){
        if(!is_authenticated(request)) return request->send(401, "text/plain", "Unauthorized");
        prefs.begin("v_mod", false);
        if(request->hasParam("ssid", true)) prefs.putString("ssid", request->getParam("ssid", true)->value());
        if(request->hasParam("pass", true)) prefs.putString("pass", request->getParam("pass", true)->value());
        if(request->hasParam("web_pass", true) && request->getParam("web_pass", true)->value().length() >= 4) prefs.putString("web_pass", request->getParam("web_pass", true)->value());
        prefs.putBool("inv_en", request->hasParam("inv_en", true));
        if(request->hasParam("inv_mac", true)) prefs.putString("inv_mac", request->getParam("inv_mac", true)->value());
        if(request->hasParam("inv_key", true)) prefs.putString("inv_key", request->getParam("inv_key", true)->value());
        prefs.putBool("sh_en", request->hasParam("sh_en", true));
        if(request->hasParam("sh_mac", true)) prefs.putString("sh_mac", request->getParam("sh_mac", true)->value());
        if(request->hasParam("sh_key", true)) prefs.putString("sh_key", request->getParam("sh_key", true)->value());
        prefs.end(); 
        
        request->send(200, "text/html", "<h3>Sparat! Startar om...</h3>"); 
        delay(1500); 
        ESP.restart();
    });

    server.begin();
    init_lvgl_interface();
}

unsigned long last_schedule_check = 0;
void loop() {
    lv_timer_handler();
    delay(5);
    update_display_dimming();
    
    if (millis() - last_schedule_check > 30000) { 
        check_relay_schedules(); 
        last_schedule_check = millis(); 
    }
}

