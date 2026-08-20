#ifndef WEBPAGES_H
#define WEBPAGES_H

#include <Arduino.h>

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
        .capsule { background: #181A1F; border: 1px solid #282C34; border-radius: 20px; padding: 15px; width: 320px; margin: 10px; }
        .value { font-size: 1.8em; color: #fff; font-weight: bold; margin-top: 5px; }
        .title { font-size: 0.9em; color: #8A92A6; text-transform: uppercase; letter-spacing: 1px; }
        .btn-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; width: 320px; margin: 15px; }
        .relay-btn { padding: 15px; border-radius: 12px; border: 1px solid #353b45; font-weight: bold; cursor: pointer; color: white; background: #282C34; }
        .relay-btn.active { background: #4CAF50; border-color: #4CAF50; }
        .nav-btn { display: inline-block; margin: 5px; padding: 10px 20px; background: #282C34; color: #fff; text-decoration: none; border-radius: 10px; }
    </style>
</head>
<body>
    <div class="status-bar"><div id="web-clock">00:00</div><div id="web-wifi">Ansluter...</div></div>
    <div class="container">
        <h2>System Dashboard (v2)</h2>
        <div id="shunt-box" class="capsule"><div class="title">🔋 SmartShunt</div><div id="shunt-val" class="value">0.00 V</div></div>
        
        <!-- DYNAMISK INVERTER SIDA/KORT -->
        <div id="inv-box" class="capsule" style="display:none; border-color: #e67e22;">
            <div class="title">⚡ Phoenix Inverter</div>
            <div id="inv-val" class="value">0 W</div>
            <div id="inv-state" style="color:#e67e22; margin-top:5px; font-weight:bold;">Off</div>
        </div>

        <h3>🔌 I2C Reläkontroll</h3>
        <div class="btn-grid">
            <button id="r0" class="relay-btn" onclick="toggleRelay(0)">Relä 1</button>
            <button id="r1" class="relay-btn" onclick="toggleRelay(1)">Relä 2</button>
            <button id="r2" class="relay-btn" onclick="toggleRelay(2)">Relä 3</button>
            <button id="r3" class="relay-btn" onclick="toggleRelay(3)">Relä 4</button>
        </div>
        <br>
        <a href="/settings" class="nav-btn">⚙️ Inställningar</a>
        <a href="/scheduler" class="nav-btn">📅 Schemaläggning</a>
    </div>
    <script>
        function toggleRelay(id) { fetch('/toggle_relay?id=' + id); }
        setInterval(function() {
            fetch('/data').then(res => res.json()).then(data => {
                document.getElementById('web-clock').innerText = data.clock;
                document.getElementById('web-wifi').innerText = data.wifi_ssid + " (" + data.wifi_rssi + " dBm)";
                document.getElementById('shunt-val').innerText = data.shunt_v + "V / " + data.shunt_a + "A";
                
                // Visa/Dölj växelriktaren baserat på om den är aktiv
                if(data.inv_active) {
                    document.getElementById('inv-box').style.display = 'block';
                    document.getElementById('inv-val').innerText = data.inv_w + " W";
                    document.getElementById('inv-state').innerText = "Status: " + data.inv_state;
                } else {
                    document.getElementById('inv-box').style.display = 'none';
                }

                for(let i=0; i<4; i++) {
                    let btn = document.getElementById('r'+i);
                    if((data.relays & (1 << i))) { btn.classList.add('active'); } else { btn.classList.remove('active'); }
                }
            });
        }, 1000);
    </script>
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
        input[type="text"], select { width: 100%; padding: 10px; background: #282C34; color: #fff; border: 1px solid #353b45; border-radius: 8px; box-sizing: border-box; }
        .btn { background: #2196F3; color: white; padding: 12px; border: none; width: 100%; border-radius: 8px; font-weight: bold; cursor: pointer; font-size: 1em; }
    </style>
</head>
<body>
    <h2>⚙️ Systemkonfiguration</h2>
    <form action="/save_advanced" method="POST">
        <div class="card">
            <h3>🌐 Nätverk</h3>
            <label>Wi-Fi SSID:</label><input type="text" name="ssid" value="%SSID%"><br><br>
            <label>Wi-Fi Lösenord:</label><input type="text" name="pass" value="%PASS%">
        </div>
        
        <!-- PHOENIX INVERTER VAL -->
        <div class="card" style="border-color: #e67e22;">
            <div class="row"><h3>⚡ Victron Phoenix Inverter</h3><label class="switch"><input type="checkbox" name="inv_en" value="1" %INV_EN%><span class="slider"></span></label></div>
            <label>MAC-Adress:</label><input type="text" name="inv_mac" value="%INV_MAC%"><br><br>
            <label>Bindkey (Hex):</label><input type="text" name="inv_key" value="%INV_KEY%">
        </div>

        <div class="card">
            <div class="row"><h3>🔋 Victron SmartShunt</h3><label class="switch"><input type="checkbox" name="sh_en" value="1" %SH_EN%><span class="slider"></span></label></div>
            <label>MAC-Adress:</label><input type="text" name="sh_mac" value="%SH_MAC%"><br><br>
            <label>Bindkey (Hex):</label><input type="text" name="sh_key" value="%SH_KEY%">
        </div>
        <div style="max-width:480px; margin:auto;"><button type="submit" class="btn">Spara och Starta om</button></div>
    </form>
</body>
</html>
)rawliteral";

extern const char scheduler_html[] PROGMEM;
#endif
