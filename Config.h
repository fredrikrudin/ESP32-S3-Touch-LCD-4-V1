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

extern Preferences prefs; extern AsyncWebServer server;
extern VictronDevice shunt; extern VictronDevice mppt; extern VictronDevice ip22; extern EcoWorthyDevice ecoBatt;
extern PhoenixInverter inverter;

extern String wifi_ssid; extern wifi_pass; extern int update_interval;
extern String web_password; // Nytt sparat lösenord för webbgränssnittet
extern byte relay_states; extern RelaySchedule relay_sched;
extern unsigned long last_touch_time; extern int current_brightness;

// Autentiserings-hjälpare
bool is_authenticated(AsyncWebServerRequest *request);

#endif
