#ifndef WEBPAGES_H
#define WEBPAGES_H

#include <Arduino.h>

// --- GUI v2 DASHBOARD HTML ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>VenusOS GUI v2</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; background-color: #0B0C0E; color: #fff; margin: 0; padding: 0; text-align: center; }
        .status-bar { background: #000; padding: 10px; display: flex; justify-content: space-between; color: #8A92A6; font-size: 0.9em; border-bottom: 1px solid #181A1F; }
        .container { display: flex; flex-direction: column; align-items: center; padding: 20px; }
        .capsule { background: #181A1F; border: 1px solid #282C34; border-radius: 20px; padding: 15px; width: 320px; margin: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
        .value { font-size: 1.8em; color: #fff; font-weight: bold; margin-top: 5px; }
        .title { font-size: 0.9em; color: #8A92A6; text-transform: uppercase; letter-spacing: 1px; }
        .nav-btn { display: inline-block; margin-top: 20px; padding: 10px 20px; background: #282C34; color: #fff; text-decoration: none; border-radius: 10px; font-size: 0.9em; }
        .nav-btn:hover { background: #353b45; }
    </style>
</head>
<body>
    <div class="status-bar">
        <div id="web-clock">00:00</div>
        <div id="web-wifi">Ansluter...</div>
    </div>
    <div class="container">
        <h2>System Dashboard (v2)</h2>
        <div class="capsule"><div class="title">☀️ MPPT Solcell</div><div id="mppt-val" class="value">0 W</div></div>
        <div class="capsule"><div class="title">🔌 IP22 Laddare</div><div id="ip22-val" class="value">0 W</div></div>
        <div class="capsule"><div class="title">🔋 SmartShunt Batteri</div><div id="shunt-val" class="value">0.00 V / 0.0 A</div></div>
        <div class="capsule"><div class="title">🔋 Eco-Worthy LiFePO4</div><div id="eco-val" class="value">0.00 V</div></div>
        <a href="/settings" class="nav-btn">⚙️ Systeminställningar</a>
    </div>
    <script>
        setInterval(function() {
            fetch('/data').then(res => res.json()).then(data => {
                document.getElementById('web-clock').innerText = data.clock;
                document.getElementById('web-wifi').innerText = data.wifi_ssid + " (" + data.wifi_rssi + " dBm)";
                document.getElementById('mppt-val').innerText = data.mppt_w + " W";
                document.getElementById('ip22-val').innerText = data.ip22_w + " W";
                document.getElementById('shunt-val').innerText = data.shunt_v + "V / " + data.shunt_a + "A / " + data.shunt_soc + "%";
                document.getElementById('eco-val').innerText = data.eco_v + "V / " + data.eco_soc + "% / " + data.eco_t + "°C";
            });
        }, 1000);
    </script>
</body>
</html>
)rawliteral";

// --- AVANCERAD INSTÄLLNINGS-HTML ---
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
        input[type="text"], select { width: 100%; padding: 10px; background: #282C34; color: #fff; border: 1px solid #353b45; border-radius: 8px; box-sizing: border-box; }
        .btn { background: #2196F3; color: white; padding: 12px; border: none; width: 100%; border-radius: 8px; font-weight: bold; cursor: pointer; font-size: 1em; }
    </style>
</head>
<body>
    <h2>⚙️ Hårdvara & System</h2>
    <form action="/save_advanced" method="POST">
        <div class="card">
            <h3>🌐 Nätverksinställningar</h3>
            <label>Wi-Fi SSID:</label><input type="text" name="ssid" value="%SSID%"><br><br>
            <label>Wi-Fi Lösenord:</label><input type="text" name="pass" value="%PASS%"><br><br>
            <label>Frekvens (sekunder):</label>
            <select name="interval">
                <option value="1" %INT1%>1 sekund</option>
                <option value="2" %INT2%>2 sekunder</option>
                <option value="5" %INT5%>5 sekunder</option>
            </select>
        </div>
        <div class="card">
            <div class="row"><h3>🔋 Victron SmartShunt</h3><label class="switch"><input type="checkbox" name="sh_en" value="1" %SH_EN%><span class="slider"></span></label></div>
            <label>MAC-Adress:</label><input type="text" name="sh_mac" value="%SH_MAC%"><br><br>
            <label>Bindkey (Hex):</label><input type="text" name="sh_key" value="%SH_KEY%">
        </div>
        <div class="card">
            <div class="row"><h3>☀️ Victron MPPT Solcell</h3><label class="switch"><input type="checkbox" name="mp_en" value="1" %MP_EN%><span class="slider"></span></label></div>
            <label>MAC-Adress:</label><input type="text" name="mp_mac" value="%MP_MAC%"><br><br>
            <label>Bindkey (Hex):</label><input type="text" name="mp_key" value="%MP_KEY%">
        </div>
        <div class="card">
            <div class="row"><h3>🔌 Victron IP22 Laddare</h3><label class="switch"><input type="checkbox" name="ip_en" value="1" %IP_EN%><span class="slider"></span></label></div>
            <label>MAC-Adress:</label><input type="text" name="ip_mac" value="%IP_MAC%"><br><br>
            <label>Bindkey (Hex):</label><input type="text" name="ip_key" value="%IP_KEY%">
        </div>
        <div class="card">
            <div class="row"><h3>🔋 Eco-Worthy LiFePO4</h3><label class="switch"><input type="checkbox" name="eco_en" value="1" %ECO_EN%><span class="slider"></span></label></div>
            <label>MAC-Adress:</label><input type="text" name="eco_mac" value="%ECO_MAC%">
        </div>
        <div style="max-width:480px; margin:auto;"><button type="submit" class="btn">Spara och Tillämpa</button></div>
    </form>
</body>
</html>
)rawliteral";

#endif
