#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <lvgl.h>

#define BACKLIGHT_PIN 1  
#define PWM_CHANNEL   0
#define PWM_FREQ      5000
#define PWM_RES       8
#define PCF8574_I2C_ADDRESS 0x20 

struct RelaySchedule { bool enabled; int start_hour; int start_min; int end_hour; int end_min; };
struct VictronDevice { String mac; String key; bool enabled; float voltage; float current; float soc; };
struct EcoWorthyDevice { String mac; bool enabled; float voltage; float current; float soc; float temp; };

// Ny struktur specifik för Phoenix Växelriktare
struct PhoenixInverter {
    String mac;
    String key;
    bool enabled;
    float battery_voltage;
    int ac_watt;
    int state_code; // 0=Off, 3=On, 4=ECO, 253=Fault etc.
    int alarm_code;
};

extern Preferences prefs; extern AsyncWebServer server;
extern VictronDevice shunt; extern VictronDevice mppt; extern VictronDevice ip22; extern EcoWorthyDevice ecoBatt;
extern PhoenixInverter inverter; // Global instans

extern String wifi_ssid; extern wifi_pass; extern int update_interval;
extern byte relay_states; extern RelaySchedule relay_sched[4];
extern unsigned long last_touch_time; extern int current_brightness;

// UI Globala Objekt
extern lv_obj_t * lbl_clock; extern lv_obj_t * lbl_wifi;
extern lv_obj_t * cb_shunt; extern lv_obj_t * cb_mppt; extern lv_obj_t * cb_ip22; extern lv_obj_t * cb_eco; extern lv_obj_t * cb_inv;
extern lv_obj_t * btn_relays[4];
extern lv_obj_t * tabview; extern lv_obj_t * tab_inverter; // Flexibla flikar

const char* get_wifi_symbol(int rssi);
const char* get_inverter_state_str(int code);
void write_relays(byte state_mask);
void check_relay_schedules();
void init_backlight();
void set_backlight(int percentage);
void update_display_dimming();

#endif
