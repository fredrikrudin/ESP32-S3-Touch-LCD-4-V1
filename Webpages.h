#ifndef WEBPAGES_H
#define WEBPAGES_H

#include <Arduino.h>

// --- NY INLOGGNINGSSIDA (GUI v2 Stil) ---
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
        .btn { background: #2196F3; color: white; padding: 12px; border: none; width: 100%; border-radius: 8px; font-weight: bold; cursor: pointer; font-size: 1em; }
        .error { color: #e74c3c; margin-bottom: 15px; font-size: 0.9em; }
    </style>
</head>
<body>
    <div class="login-card">
        <h2>🔒 VenusOS Login</h2>
        <p style="color: #8A92A6; font-size: 0.9em;">Ange autentiseringsuppgifter</p>
        <form action="/login" method="POST">
            <input type="text" name="username" placeholder="Användarnamn (admin)" required>
            <input type="password" name="password" placeholder="Lösenord" required>
            <button type="submit" class="btn">Logga in</button>
        </form>
    </div>
</body>
</html>
)rawliteral";

// --- UPPDATERAD INSTÄLLNINGSSIDA (Inkluderar nytt lösenordsfält) ---
const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>VenusOS v2 - Inställningar</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; background: #0B0C0E; color: #fff; padding: 20px; text-align: center; }
        .card { background: #181A1F; border: 1px solid #282C34; padding: 20px; border-radius: 15px; max-width: 480px; margin: 15px auto; text-align: left; }
        input[type="text"], input[type="password"] { width: 100%; padding: 10px; background: #282C34; color: #fff; border: 1px solid #353b45; border-radius: 8px; box-sizing: border-box; }
        .btn { background: #2196F3; color: white; padding: 12px; border: none; width: 100%; border-radius: 8px; font-weight: bold; cursor: pointer; }
    </style>
</head>
<body>
    <h2>⚙️ Systemkonfiguration</h2>
    <form action="/save_advanced" method="POST">
        <div class="card">
            <h3>🌐 Nätverk & Wi-Fi</h3>
            <label>Wi-Fi SSID:</label><input type="text" name="ssid" value="%SSID%"><br><br>
            <label>Wi-Fi Lösenord:</label><input type="password" name="pass" value="%PASS%">
        </div>

        <!-- NY SEKTION: SÄKERHET -->
        <div class="card" style="border-color: #2196F3;">
            <h3>🔒 Webbsäkerhet</h3>
            <label>Nytt administrationslösenord:</label>
            <input type="password" name="web_pass" value="%WEBPASS%" placeholder="Minst 4 tecken">
            <p style="font-size:0.8em; color:#8A92A6; margin-top:5px;">Användarnamnet är alltid 'admin'.</p>
        </div>
        
        <div class="card">
            <h3>🔋 Victron SmartShunt</h3>
            <label>MAC-Adress:</label><input type="text" name="sh_mac" value="%SH_MAC%">
        </div>
        <div style="max-width:480px; margin:auto;"><button type="submit" class="btn">Spara och Starta om</button></div>
    </form>
</body>
</html>
)rawliteral";

extern const char index_html[] PROGMEM;
extern const char scheduler_html[] PROGMEM;
#endif
