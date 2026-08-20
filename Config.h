#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <lvgl.h>

// Datastrukturer för enheter
struct VictronDevice {
    String mac;
    String key;
    bool enabled;
    float voltage;
    float current;
    float soc;
};

struct EcoWorthyDevice {
    String mac;
    bool enabled;
    float voltage;
    float current;
    float soc;
    float temp;
};

// Globala instanser (görs tillgängliga för alla filer via extern)
extern Preferences prefs;
extern AsyncWebServer server;

extern VictronDevice shunt;
extern VictronDevice mppt;
extern VictronDevice ip22;
extern EcoWorthyDevice ecoBatt;

extern String wifi_ssid;
extern String wifi_pass;
extern int update_interval;

// UI Globala Objekt
extern lv_obj_t * lbl_clock;
extern lv_obj_t * lbl_wifi;
extern lv_obj_t * cb_shunt;
extern lv_obj_t * cb_mppt;
extern lv_obj_t * cb_ip22;
extern lv_obj_t * cb_eco;

const char* get_wifi_symbol(int rssi);

#endif
