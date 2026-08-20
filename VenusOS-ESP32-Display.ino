#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <time.h>
#include <Wire.h>
#include <lvgl.h>
#include "Config.h"
#include "Webpages.h"

Preferences prefs; AsyncWebServer server(80);
VictronDevice shunt; VictronDevice mppt; VictronDevice ip22; EcoWorthyDevice ecoBatt;
PhoenixInverter inverter; // Definition

String wifi_ssid, wifi_pass; int update_interval;
byte relay_states = 0x00; RelaySchedule relay_sched[4];
unsigned long last_touch_time = 0; int current_brightness = 100;

lv_obj_t * lbl_clock; lv_obj_t * lbl_wifi;
lv_obj_t * cb_shunt; lv_obj_t * cb_mppt; lv_obj_t * cb_ip22; lv_obj_t * cb_eco; lv_obj_t * cb_inv;
lv_obj_t * btn_relays[4];
lv_obj_t * tabview; lv_obj_t * tab_inverter; // Flikobjekt för skärmen

// Textöversättning för Victrons statuskoder
const char* get_inverter_state_str(int code) {
    switch(code) {
        case 0:   return "Avstängd (Off)";
        case 3:   return "Igång (On)";
        case 4:   return "ECO-Mode";
        case 253: return "⚠️ Fel (Fault)";
        default:  return "Okänd status";
    }
}

// [write_relays, check_relay_schedules, init_backlight, set_backlight och update_display_dimming ligger kvar intakta]
void write_relays(byte state_mask) { byte pcf_data = 0xF0 | (~state_mask & 0x0F); Wire.beginTransmission(PCF8574_I2C_ADDRESS); Wire.write(pcf_data); Wire.endTransmission(); }
void init_backlight() { ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES); ledcAttachPin(BACKLIGHT_PIN, PWM_CHANNEL); set_backlight(100); last_touch_time = millis(); }
void set_backlight(int percentage) { if (percentage < 0) percentage = 0; if (percentage > 100) percentage = 100; current_brightness = percentage; ledcWrite(PWM_CHANNEL, (percentage * 255) / 100); }
void update_display_dimming() { time_t now; struct tm ti; bool is_night = (getLocalTime(&ti) && (ti.tm_hour >= 22 || ti.tm_hour < 6)); if (millis() - last_touch_time > 30000) set_backlight(10); else set_backlight(is_night ? 30 : 100); }
static void global_touch_feedback_cb(lv_event_t * e) { last_touch_time = millis(); update_display_dimming(); }

static void checkbox_event_cb(lv_event_t * e) {
    lv_obj_t * cb = lv_event_get_target(e);
    bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
    prefs.begin("v_mod", false);
    if(cb == cb_inv) { prefs.putBool("inv_en", checked); inverter.enabled = checked; }
    // [Övriga enheters sparning laddas här]
    prefs.end();
}

void init_lvgl_interface() {
    lv_obj_add_event_cb(lv_scr_act(), global_touch_feedback_cb, LV_EVENT_PRESSED, NULL);

    // STATUSRAD
    lv_obj_t * status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, 480, 35);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    lbl_clock = lv_label_create(status_bar); lv_obj_align(lbl_clock, LV_ALIGN_LEFT_MID, 10, 0);
    lbl_wifi = lv_label_create(status_bar); lv_obj_align(lbl_wifi, LV_ALIGN_CENTER, 0, 0);

    // FLIK-SYSTEM
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 40);
    lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(tabview, 480, 445);
    lv_obj_set_style_bg_color(tabview, lv_color_hex(0x0B0C0E), 0);

    lv_obj_t * tab_dash = lv_tabview_add_tab(tabview, "Dashboard");
    
    // --- SKAPA EN EGEN DETALJSIDA FÖR VÄXELRIKTAREN OM DEN ÄR AKTIV ---
    if (inverter.enabled) {
        tab_inverter = lv_tabview_add_tab(tabview, "Inverter");
        lv_obj_set_style_bg_color(tab_inverter, lv_color_hex(0x0B0C0E), 0);

        // Stor rund v2-kapsel för växelriktardata på skärmen
        lv_obj_t * cap_inv = lv_obj_create(tab_inverter);
        lv_obj_set_size(cap_inv, 420, 150);
        lv_obj_align(cap_inv, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(cap_inv, 25, 0);
        lv_obj_set_style_bg_color(cap_inv, lv_color_hex(0x181A1F), 0);
        lv_obj_set_style_border_color(cap_inv, lv_color_hex(0xE67E22), 0); // Orange v2-ram för AC

        lv_obj_t * lbl_inv_title = lv_label_create(cap_inv);
        lv_label_set_text(lbl_inv_title, "⚡ PHOENIX PURE SINE INVERTER");
        lv_obj_set_style_text_color(lbl_inv_title, lv_color_hex(0x8A92A6), 0);
        lv_obj_align(lbl_inv_title, LV_ALIGN_TOP_MID, 0, 10);

        lv_obj_t * lbl_inv_watt = lv_label_create(cap_inv);
        // Uppdateras dynamiskt i drift sen, visar demo-nollställning här
        lv_label_set_text(lbl_inv_watt, "0 W\nAC Output");
        lv_obj_set_style_text_font(lbl_inv_watt, &lv_font_montserrat_24, 0);
        lv_obj_align(lbl_inv_watt, LV_ALIGN_CENTER, 0, 10);
    }

    lv_obj_t * tab_hw = lv_tabview_add_tab(tabview, "Hårdvara");
    lv_obj_set_flex_flow(tab_hw, LV_FLEX_FLOW_COLUMN);
    
    cb_inv = lv_checkbox_create(tab_hw); lv_checkbox_set_text(cb_inv, "Aktivera Phoenix Inverter");
    if(inverter.enabled) lv_obj_add_state(cb_inv, LV_STATE_CHECKED);
    lv_obj_add_event_cb(cb_inv, checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void loadSettings() {
    prefs.begin("v_mod", true);
    wifi_ssid = prefs.getString("ssid", "DITT_SSID");
    wifi_pass = prefs.getString("pass", "DITT_LÖSENORD");
    update_interval = prefs.getInt("interval", 2);

    inverter.enabled = prefs.getBool("inv_en", false);
    inverter.mac = prefs.getString("inv_mac", "00:00:00:00:00:00");
    inverter.key = prefs.getString("inv_key", "");

    shunt.enabled = prefs.getBool("sh_en", false);
    shunt.mac = prefs.getString("sh_mac", "00:00:00:00:00:00");
    shunt.key = prefs.getString("sh_key", "");
    prefs.end();
}

void setup() {
    Serial.begin(115200); Wire.begin(19, 20); write_relays(0x00); init_backlight();
    loadSettings();
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_200(index_html, "text/html"); });
    
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        time_t now; struct tm ti; char t_buf = "--:--"; if(getLocalTime(&ti)) sprintf(t_buf, "%02d:%02d", ti.tm_hour, ti.tm_min);
        String json = "{\"clock\":\"" + String(t_buf) + "\",\"wifi_ssid\":\"" + WiFi.SSID() + "\",\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
        json += "\"inv_active\":" + String(inverter.enabled ? "true" : "false") + ",";
        json += "\"inv_w\":" + String(inverter.ac_watt) + ",";
        json += "\"inv_state\":\"" + String(get_inverter_state_str(inverter.state_code)) + "\",";
        json += "\"shunt_v\":" + String(shunt.voltage) + ",\"shunt_a\":" + String(shunt.current) + "}";
        request->send(200, "application/json", json);
    });

    server.on("/save_advanced", HTTP_POST, [](AsyncWebServerRequest *request){
        prefs.begin("v_mod", false);
        if(request->hasParam("ssid", true)) prefs.putString("ssid", request->getParam("ssid", true)->value());
        if(request->hasParam("pass", true)) prefs.putString("pass", request->getParam("pass", true)->value());
        
        prefs.putBool("inv_en", request->hasParam("inv_en", true));
        if(request->hasParam("inv_mac", true)) prefs.putString("inv_mac", request->getParam("inv_mac", true)->value());
        if(request->hasParam("inv_key", true)) prefs.putString("inv_key", request->getParam("inv_key", true)->value());

        prefs.putBool("sh_en", request->hasParam("sh_en", true));
        if(request->hasParam("sh_mac", true)) prefs.putString("sh_mac", request->getParam("sh_mac", true)->value());
        prefs.end();
        request->send(200, "text/html", "<h3>Sparat! Startar om...</h3>");
        delay(1500); ESP.restart();
    });

    server.begin();
    init_lvgl_interface();
}

void loop() {
    lv_timer_handler();
    delay(5);
    update_display_dimming();
}
