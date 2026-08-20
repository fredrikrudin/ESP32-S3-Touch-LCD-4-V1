#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <time.h>
#include <lvgl.h>
#include "Config.h"
#include "Webpages.h"

// Definition av globala objekt
Preferences prefs;
AsyncWebServer server(80);

VictronDevice shunt;
VictronDevice mppt;
VictronDevice ip22;
EcoWorthyDevice ecoBatt;

String wifi_ssid, wifi_pass;
int update_interval;

lv_obj_t * lbl_clock;
lv_obj_t * lbl_wifi;
lv_obj_t * cb_shunt;
lv_obj_t * cb_mppt;
lv_obj_t * cb_ip22;
lv_obj_t * cb_eco;

const char* get_wifi_symbol(int rssi) {
    if (rssi >= -50) return "📶 [Utmärkt]";
    if (rssi >= -70) return "📶 [Bra]";
    if (rssi >= -85) return "📶 [Svag]";
    return "❌ [Ingen signal]";
}

String advancedProcessor(const String& var) {
    if(var == "SSID") return wifi_ssid;
    if(var == "PASS") return wifi_pass;
    if(var == "INT1") return (update_interval == 1) ? "selected" : "";
    if(var == "INT2") return (update_interval == 2) ? "selected" : "";
    if(var == "INT5") return (update_interval == 5) ? "selected" : "";
    if(var == "SH_EN")  return shunt.enabled ? "checked" : "";
    if(var == "SH_MAC") return shunt.mac;
    if(var == "SH_KEY") return shunt.key;
    if(var == "MP_EN")  return mppt.enabled ? "checked" : "";
    if(var == "MP_MAC") return mppt.mac;
    if(var == "MP_KEY") return mppt.key;
    if(var == "IP_EN")  return ip22.enabled ? "checked" : "";
    if(var == "IP_MAC") return ip22.mac;
    if(var == "IP_KEY") return ip22.key;
    if(var == "ECO_EN")  return ecoBatt.enabled ? "checked" : "";
    if(var == "ECO_MAC") return ecoBatt.mac;
    return String();
}

void loadSettings() {
    prefs.begin("v_mod", true);
    wifi_ssid = prefs.getString("ssid", "DITT_SSID");
    wifi_pass = prefs.getString("pass", "DITT_LÖSENORD");
    update_interval = prefs.getInt("interval", 2);
    shunt.enabled = prefs.getBool("sh_en", false);
    shunt.mac = prefs.getString("sh_mac", "00:00:00:00:00:00");
    shunt.key = prefs.getString("sh_key", "");
    mppt.enabled = prefs.getBool("mp_en", false);
    mppt.mac = prefs.getString("mp_mac", "00:00:00:00:00:00");
    mppt.key = prefs.getString("mp_key", "");
    ip22.enabled = prefs.getBool("ip_en", false);
    ip22.mac = prefs.getString("ip_mac", "00:00:00:00:00:00");
    ip22.key = prefs.getString("ip_key", "");
    ecoBatt.enabled = prefs.getBool("eco_en", false);
    ecoBatt.mac = prefs.getString("eco_mac", "00:00:00:00:00:00");
    prefs.end();
}

static void status_bar_update_cb(lv_timer_t * timer) {
    time_t now; struct tm ti;
    if (getLocalTime(&ti)) lv_label_set_text_fmt(lbl_clock, "%02d:%02d", ti.tm_hour, ti.tm_min);
    if (WiFi.status() == WL_CONNECTED) {
        lv_label_set_text_fmt(lbl_wifi, "%s  %s (%d dBm)", get_wifi_symbol(WiFi.RSSI()), WiFi.SSID().c_str(), WiFi.RSSI());
    } else {
        lv_label_set_text(lbl_wifi, "❌ Frånkopplad");
    }
}

static void checkbox_event_cb(lv_event_t * e) {
    lv_obj_t * cb = lv_event_get_target(e);
    bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
    prefs.begin("v_mod", false);
    if(cb == cb_shunt) { prefs.putBool("sh_en", checked); shunt.enabled = checked; }
    if(cb == cb_mppt)  { prefs.putBool("mp_en", checked); mppt.enabled = checked; }
    if(cb == cb_ip22)  { prefs.putBool("ip_en", checked); ip22.enabled = checked; }
    if(cb == cb_eco)   { prefs.putBool("eco_en", checked); ecoBatt.enabled = checked; }
    prefs.end();
}

void init_lvgl_interface() {
    lv_obj_t * status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, 480, 35);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_pad_all(status_bar, 5, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    lbl_clock = lv_label_create(status_bar);
    lv_label_set_text(lbl_clock, "--:--");
    lv_obj_set_style_text_color(lbl_clock, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_clock, LV_ALIGN_LEFT_MID, 10, 0);

    lbl_wifi = lv_label_create(status_bar);
    lv_label_set_text(lbl_wifi, "Ansluter...");
    lv_obj_set_style_text_color(lbl_wifi, lv_color_hex(0x8A92A6), 0);
    lv_obj_set_style_text_font(lbl_wifi, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_wifi, LV_ALIGN_CENTER, 0, 0);

    lv_timer_create(status_bar_update_cb, 1000, NULL);

    lv_obj_t * tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 40);
    lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(tabview, 480, 445);
    lv_obj_set_style_bg_color(tabview, lv_color_hex(0x0B0C0E), 0);

    lv_obj_t * tab_dash = lv_tabview_add_tab(tabview, "Dashboard");
    lv_obj_t * tab_hw = lv_tabview_add_tab(tabview, "Hårdvara");

    lv_obj_t * cap_shunt = lv_obj_create(tab_dash);
    lv_obj_set_size(cap_shunt, 400, 70);
    lv_obj_align(cap_shunt, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_radius(cap_shunt, 20, 0);
    lv_obj_set_style_bg_color(cap_shunt, lv_color_hex(0x181A1F), 0);

    lv_obj_t * lbl_sh_title = lv_label_create(cap_shunt);
    lv_label_set_text(lbl_sh_title, "🔋 SmartShunt Central");
    lv_obj_set_style_text_color(lbl_sh_title, lv_color_hex(0x8A92A6), 0);
    lv_obj_align(lbl_sh_title, LV_ALIGN_TOP_LEFT, 10, 5);

    lv_obj_set_flex_flow(tab_hw, LV_FLEX_FLOW_COLUMN);
    cb_shunt = lv_checkbox_create(tab_hw); lv_checkbox_set_text(cb_shunt, "Aktivera SmartShunt");
    if(shunt.enabled) lv_obj_add_state(cb_shunt, LV_STATE_CHECKED);
    lv_obj_add_event_cb(cb_shunt, checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    cb_mppt = lv_checkbox_create(tab_hw); lv_checkbox_set_text(cb_mppt, "Aktivera MPPT Solcell");
    if(mppt.enabled) lv_obj_add_state(cb_mppt, LV_STATE_CHECKED);
    lv_obj_add_event_cb(cb_mppt, checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    cb_ip22 = lv_checkbox_create(tab_hw); lv_checkbox_set_text(cb_ip22, "Aktivera IP22 Laddare");
    if(ip22.enabled) lv_obj_add_state(cb_ip22, LV_STATE_CHECKED);
    lv_obj_add_event_cb(cb_ip22, checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    cb_eco = lv_checkbox_create(tab_hw); lv_checkbox_set_text(cb_eco, "Aktivera Eco-Worthy");
    if(ecoBatt.enabled) lv_obj_add_state(cb_eco, LV_STATE_CHECKED);
    lv_obj_add_event_cb(cb_eco, checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void setup() {
    Serial.begin(115200);
    loadSettings();

    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 12) { delay(500); timeout++; }

    if (WiFi.status() == WL_CONNECTED) {
        configTime(3600, 3600, "0.se.pool.ntp.org", "1.se.pool.ntp.org");
    } else {
        WiFi.softAP("VenusOS-ESP32-Setup");
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_200(index_html, "text/html"); });
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_200(settings_html, "text/html", advancedProcessor); });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        time_t now; struct tm ti; char t_buf[6] = "--:--";
        if(getLocalTime(&ti)) sprintf(t_buf, "%02d:%02d", ti.tm_hour, ti.tm_min);
        String json = "{\"clock\":\"" + String(t_buf) + "\",\"wifi_ssid\":\"" + WiFi.SSID() + "\",\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
        json += "\"mppt_w\":" + String(mppt.voltage * mppt.current) + ",\"ip22_w\":" + String(ip22.voltage * ip22.current) + ",";
        json += "\"shunt_v\":" + String(shunt.voltage) + ",\"shunt_a\":" + String(shunt.current) + ",\"shunt_soc\":" + String(shunt.soc) + ",";
        json += "\"eco_v\":" + String(ecoBatt.voltage) + ",\"eco_soc\":" + String(ecoBatt.soc) + ",\"eco_t\":" + String(ecoBatt.temp) + "}";
        request->send(200, "application/json", json);
    });

    server.on("/save_advanced", HTTP_POST, [](AsyncWebServerRequest *request){
        prefs.begin("v_mod", false);
        if(request->hasParam("ssid", true)) prefs.putString("ssid", request->getParam("ssid", true)->value());
        if(request->hasParam("pass", true)) prefs.putString("pass", request->getParam("pass", true)->value());
        if(request->hasParam("interval", true)) prefs.putInt("interval", request->getParam("interval", true)->value().toInt());
        prefs.putBool("sh_en", request->hasParam("sh_en", true));
        if(request->hasParam("sh_mac", true)) prefs.putString("sh_mac", request->getParam("sh_mac", true)->value());
        if(request->hasParam("sh_key", true)) prefs.putString("sh_key", request->getParam("sh_key", true)->value());
        prefs.putBool("mp_en", request->hasParam("mp_en", true));
        if(request->hasParam("mp_mac", true)) prefs.putString("mp_mac", request->getParam("mp_mac", true)->value());
        if(request->hasParam("mp_key", true)) prefs.putString("mp_key", request->getParam("mp_key", true)->value());
        prefs.putBool("ip_en", request->hasParam("ip_en", true));
        if(request->hasParam("ip_mac", true)) prefs.putString("ip_mac", request->getParam("ip_mac", true)->value());
        if(request->hasParam("ip_key", true)) prefs.putString("ip_key", request->getParam("ip_key", true)->value());
        prefs.putBool("eco_en", request->hasParam("eco_en", true));
        if(request->hasParam("eco_mac", true)) prefs.putString("eco_mac", request->getParam("eco_mac", true)->value());
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
}
