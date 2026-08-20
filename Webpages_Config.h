#ifndef WEBPAGES_CONFIG_H
#define WEBPAGES_CONFIG_H

#include <Arduino.h>

const char scheduler_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>VenusOS v2 - Timer</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; background: #0B0C0E; color: #fff; padding: 20px; text-align: center; }
        .card { background: #181A1F; border: 1px solid #282C34; padding: 15px; border-radius: 15px; max-width: 450px; margin: 15px auto; text-align: left; }
        .row { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; }
        input[type="number"] { width: 60px; padding: 8px; background: #282C34; color: #fff; border: 1px solid #353b45; border-radius: 6px; text-align: center; }
        .btn { background: #2196F3; color: white; padding: 12px; border: none; width: 100%; border-radius: 8px; font-weight: bold; cursor: pointer; }
    </style>
</head>
<body>
    <h2>📅 Reläschemaläggning</h2>
    <form action="/save_schedule" method="POST">
        %SCHED_CARDS%
        <button type="submit" class="btn">Spara Scheman</button>
    </form>
    <br><a href="/" style="color:#2196F3; text-decoration:none;">&larr; Tillbaka</a>
</body>
</html>
)rawliteral";

const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>VenusOS v2 - Inställningar</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; background: #0B0C0E; color: #fff; padding: 20px; text-align: center; }
        .card { background: #181A1F; border: 1px solid #282C34; padding: 20px; border-radius: 15px; max-width: 480px; margin: 15px auto; text-align: left; }
        .row { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; }
        .switch { position: relative; display: inline-block; width: 44px; height: 24px; }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #353b45; transition: .3s; border-radius: 24px; }
        .slider:before { position: absolute; content: ""; height: 16px; width: 16px; left: 4px; bottom: 4px; background-color: white; transition: .3s; border-radius: 50%; }
        input:checked + .slider { background-color: #2196F3; }
        input:checked + .slider:before { transform: translateX(20px); }
        input[type="text"], input[type="password"] { width: 100%; padding: 10px; background: #282C34; color: #fff; border: 1px solid #353b45; border-radius: 8px; box-sizing: border-box; }
        .btn { background: #2196F3; color: white; padding: 12px; border: none; width: 100%; border-radius: 8px; font-weight: bold; cursor: pointer; }
    </style>
</head>
<body>
    <h2>⚙️ Systemkonfiguration</h2>
    <form action="/save_advanced" method="POST">
        <div class="card">
            <h3>🌐 Wi-Fi & System</h3>
            <label>SSID:</label><input type="text" name="ssid" value="%SSID%"><br><br>
            <label>Lösenord:</label><input type="password" name="pass" value="%PASS%"><br><br>
            <label>Pollning / Dataintervall:</label>
            <select name="interval">
                <option value="1" %INT1%>1 sekund (Hög prestanda)</option>
                <option value="10" %INT10%>10 sekunder (Balanserad)</option>
                <option value="30" %INT30%>30 sekunder (Strömspar)</option>
                <option value="60" %INT60%>60 sekunder (Max strömspar)</option>
            </select>
        </div>
        <div class="card" style="border-color: #2196F3;">
            <h3>🔒 Webbsäkerhet</h3>
            <label>Nytt admin-lösenord:</label>
            <input type="password" name="web_pass" value="%WEBPASS%">
        </div>
        <div class="card" style="border-color: #e67e22;">
            <div class="row"><h3>⚡ Phoenix Inverter</h3><label class="switch"><input type="checkbox" name="inv_en" value="1" %INV_EN%><span class="slider"></span></label></div>
            <label>MAC:</label><input type="text" name="inv_mac" value="%INV_MAC%"><br><br>
            <label>Bindkey:</label><input type="text" name="inv_key" value="%INV_KEY%">
        </div>
        <div class="card">
            <div class="row"><h3>🔋 SmartShunt</h3><label class="switch"><input type="checkbox" name="sh_en" value="1" %SH_EN%><span class="slider"></span></label></div>
            <label>MAC:</label><input type="text" name="sh_mac" value="%SH_MAC%"><br><br>
            <label>Bindkey:</label><input type="text" name="sh_key" value="%SH_KEY%">
        </div>
        <button type="submit" class="btn">Spara & Starta om</button>
    </form>
</body>
</html>
)rawliteral";

#endif
