#ifndef WEBPAGES_CORE_H
#define WEBPAGES_CORE_H

#include <Arduino.h>

const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>VenusOS - Logga in</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; background: #0B0C0E; color: #fff; padding: 20px; text-align: center; }
        .login-card { background: #181A1F; border: 1px solid #282C34; padding: 30px; border-radius: 20px; max-width: 360px; margin: 80px auto; box-shadow: 0 4px 10px rgba(0,0,0,0.5); }
        input[type="text"], input[type="password"] { width: 100%; padding: 12px; background: #282C34; color: #fff; border: 1px solid #353b45; border-radius: 8px; box-sizing: border-box; margin-bottom: 15px; }
        .btn { background: #2196F3; color: white; padding: 12px; border: none; width: 100%; border-radius: 8px; font-weight: bold; cursor: pointer; }
    </style>
</head>
<body>
    <div class="login-card">
        <h2>🔒 VenusOS Login</h2>
        <p style="color: #8A92A6; font-size: 0.9em;">Användarnamn: admin</p>
        <form action="/login" method="POST">
            <input type="text" name="username" placeholder="Användarnamn" required>
            <input type="password" name="password" placeholder="Lösenord" required>
            <button type="submit" class="btn">Logga in</button>
        </form>
    </div>
</body>
</html>
)rawliteral";

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
        .title { font-size: 0.9em; color: #8A92A6; text-transform: uppercase; }
        .btn-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; width: 320px; margin: 15px; }
        .relay-btn { padding: 15px; border-radius: 12px; border: 1px solid #353b45; font-weight: bold; cursor: pointer; color: white; background: #282C34; }
        .relay-btn.active { background: #4CAF50; border-color: #4CAF50; }
        .nav-btn { display: inline-block; margin: 5px; padding: 10px 20px; background: #282C34; color: #fff; text-decoration: none; border-radius: 10px; font-size: 0.9em; }
    </style>
</head>
<body>
    <div class="status-bar"><div id="web-clock">00:00</div><div id="web-wifi">Ansluter...</div></div>
    <div class="container">
        <h2>System Dashboard (v2)</h2>
        <div id="shunt-box" class="capsule"><div class="title">🔋 SmartShunt</div><div id="shunt-val" class="value">0.00 V</div></div>
        <div id="inv-box" class="capsule" style="display:none; border-color:#e67e22;"><div class="title">⚡ Inverter</div><div id="inv-val" class="value">0 W</div><div id="inv-state">Off</div></div>
        <h3>🔌 I2C Reläkontroll</h3>
        <div class="btn-grid">
            <button id="r0" class="relay-btn" onclick="toggleRelay(0)">Relä 1</button>
            <button id="r1" class="relay-btn" onclick="toggleRelay(1)">Relä 2</button>
            <button id="r2" class="relay-btn" onclick="toggleRelay(2)">Relä 3</button>
            <button id="r3" class="relay-btn" onclick="toggleRelay(3)">Relä 4</button>
        </div>
        <a href="/settings" class="nav-btn">⚙️ Inställningar</a>
        <a href="/scheduler" class="nav-btn">📅 Timer</a>
    </div>
    <script>
        function toggleRelay(id) { fetch('/toggle_relay?id=' + id); }
        setInterval(function() {
            fetch('/data').then(res => res.json()).then(data => {
                document.getElementById('web-clock').innerText = data.clock;
                document.getElementById('web-wifi').innerText = data.wifi_ssid + " (" + data.wifi_rssi + " dBm)";
                document.getElementById('shunt-val').innerText = data.shunt_v + "V / " + data.shunt_a + "A";
                if(data.inv_active) {
                    document.getElementById('inv-box').style.display = 'block';
                    document.getElementById('inv-val').innerText = data.inv_w + " W";
                    document.getElementById('inv-state').innerHTML = "Status: " + data.inv_state + "<br><span style='color:#e74c3c;'>" + data.inv_alarm + "</span>";
                } else { document.getElementById('inv-box').style.display = 'none'; }
                for(let i=0; i<4; i++) {
                    let btn = document.getElementById('r'+i);
                    if((data.relays & (1 << i))) btn.classList.add('active'); else btn.classList.remove('active');
                }
            });
        }, 1000);
    </script>
</body>
</html>
)rawliteral";

#endif
