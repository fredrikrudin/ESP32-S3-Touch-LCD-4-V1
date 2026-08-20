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
struct PhoenixInverter { String mac; String key; bool enabled; float battery_voltage; int ac_watt; int state_code; int alarm_code; };

extern Preferences prefs; 
extern AsyncWebServer server;
extern VictronDevice shunt; extern VictronDevice mppt; extern VictronDevice ip22; extern EcoWorthyDevice ecoBatt;
extern PhoenixInverter inverter;

extern String wifi_ssid; extern String wifi_pass; extern String web_password; 
extern int update_interval; // Sparat pollningsintervall (1, 10, 30 eller 60 sekunder)
extern byte relay_states; extern RelaySchedule relay_sched[4];
extern unsigned long last_touch_time; extern int current_brightness;

bool is_authenticated(AsyncWebServerRequest *request);
const char* get_wifi_symbol(int rssi);
const char* get_inverter_state_str(int code);
void write_relays(byte state_mask);
void check_relay_schedules();
void init_backlight();
void set_backlight(int percentage);
void update_display_dimming();

#endif
