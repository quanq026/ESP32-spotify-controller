#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FontMaker.h"
#include <WebServer.h>
#include <Preferences.h>

// ======================
// WiFi + Spotify Config
// ======================
const char* AP_SSID = "ESP32C3_Config";
const char* AP_PASS = "12345678";
String wifiSsid = "";
String wifiPass = "";

const char* CLIENT_ID     = "your_client_id";
const char* CLIENT_SECRET = "your_client_secret";

const char* SPOTIFY_CURRENTLY_PLAYING_URL = "/me/player/currently-playing";

// ======================
// OLED
// ======================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setPixel(int16_t x, int16_t y, uint16_t color) {
    if (color) display.drawPixel(x, y, SSD1306_WHITE);
    else       display.drawPixel(x, y, SSD1306_BLACK);
}
MakeFont customFont(&setPixel);

// ======================
// Track State
// ======================
String songTitle = "Loading...";
String artist = "";
int totalSeconds = 0;
int currentSeconds = 0;
String currentTrackId = "";
bool isTrackLiked = false;
bool isPlaying = false;
bool isBackEnabled = false;
bool isNextEnabled = false;

// ======================
// Scrolling
// ======================
int titleOffset = 0;
int artistOffset = 0;
unsigned long lastScrollTime = 0;
const unsigned long SCROLL_INTERVAL_MS = 15;
unsigned long lastFetchTime = 0;
const unsigned long FETCH_INTERVAL_MS = 500; // 2s để đỡ spam

// ======================
// Buttons
// ======================
#define BUTTON_PREV   2
#define BUTTON_TOGGLE 3
#define BUTTON_NEXT   4
#define BUTTON_LIKE   5
const unsigned long DEBOUNCE_MS = 150;
unsigned long lastButtonPress[4] = {0};
bool lastToggleButtonState = HIGH;

// ======================
// WebServer + Preferences
// ======================
WebServer server(80);
Preferences preferences;

// ======================
// Token Manager
// ======================
void save_token(String access, String refresh) {
    preferences.begin("spotify", false);
    String oldData = preferences.getString("tokens", "");
    String entry   = "a.token:" + access + "\n" +
                     "r.token:" + refresh + "\n***\n";
    preferences.putString("tokens", oldData + entry);
    preferences.end();
    Serial.println("[SAVE] Token pair added.");
}

String get_tokens_raw() {
    preferences.begin("spotify", true);
    String all = preferences.getString("tokens", "");
    preferences.end();
    return all;
}

String read_lastest_access_token() {
    String all = get_tokens_raw();
    int lastPos = all.lastIndexOf("a.token:");
    if (lastPos < 0) return "";
    int endPos = all.indexOf("\n", lastPos);
    String token = all.substring(lastPos + 8, endPos);
    return token;
}

String read_lastest_refresh_token() {
    String all = get_tokens_raw();
    int lastPos = all.lastIndexOf("r.token:");
    if (lastPos < 0) return "";
    int endPos = all.indexOf("\n", lastPos);
    String token = all.substring(lastPos + 8, endPos);
    return token;
}

void renew_token() {
    String refresh = read_lastest_refresh_token();
    if (refresh == "") {
        Serial.println("[ERR] No refresh token.");
        return;
    }

    HTTPClient http;
    String url = "https://accounts.spotify.com/api/token";
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String body = "grant_type=refresh_token";
    body += "&refresh_token=" + refresh;
    body += "&client_id=" + String(CLIENT_ID);
    body += "&client_secret=" + String(CLIENT_SECRET);

    int code = http.POST(body);
    if (code != 200) {
        Serial.printf("[ERR] renew_token HTTP %d\n", code);
        http.end();
        return;
    }

    String payload = http.getString();
    http.end();
    Serial.println("[RESP] " + payload);

    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, payload) != DeserializationError::Ok) {
        Serial.println("[ERR] JSON parse fail.");
        return;
    }

    String newAccess  = doc["access_token"] | "";
    String newRefresh = doc["refresh_token"] | refresh;

    if (newAccess != "") {
        save_token(newAccess, newRefresh);
        Serial.println("[OK] Token renewed.");
    } else {
        Serial.println("[ERR] No access_token in response.");
    }
}

// ======================
// HTTP Wrapper with auto-refresh
// ======================
int spotifyRequest(const char* method, const String& path, String& response, const String& body = "") {
    String access = read_lastest_access_token();
    HTTPClient http;
    String url = "https://api.spotify.com/v1" + path;

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + access);
    if (strcmp(method, "PUT") == 0 || strcmp(method, "POST") == 0)
        http.addHeader("Content-Length", String(body.length()));

    int code = -1;
    if (strcmp(method, "GET") == 0) { 
        code = http.GET(); 
        if (code == 200) response = http.getString(); 
    }
    else if (strcmp(method, "PUT") == 0)  code = http.PUT(body);
    else if (strcmp(method, "POST") == 0) code = http.POST(body);
    else if (strcmp(method, "DELETE") == 0) code = http.sendRequest("DELETE", body);

    http.end();

    // Retry khi 401
    if (code == 401) {
        Serial.println("[WARN] Token expired → Renewing...");
        renew_token();
        access = read_lastest_access_token();

        HTTPClient retry;
        retry.begin(url);
        retry.addHeader("Authorization", "Bearer " + access);
        if (strcmp(method, "PUT") == 0 || strcmp(method, "POST") == 0)
            retry.addHeader("Content-Length", String(body.length()));

        if (strcmp(method, "GET") == 0) { 
            code = retry.GET(); 
            if (code == 200) response = retry.getString(); 
        }
        else if (strcmp(method, "PUT") == 0)  code = retry.PUT(body);
        else if (strcmp(method, "POST") == 0) code = retry.POST(body);
        else if (strcmp(method, "DELETE") == 0) code = retry.sendRequest("DELETE", body);

        retry.end();
    }

    Serial.printf("[%s] %s -> %d\n", method, path.c_str(), code);
    return code;
}

// ======================
// Web UI
// ======================
void handleRoot() {
    String latestAccess  = read_lastest_access_token();
    String latestRefresh = read_lastest_refresh_token();

    String html = R"rawliteral(
    <!DOCTYPE html><html><head>
        <title>ESP32-C3 Spotify Config</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>body{font-family:Arial;text-align:center;padding:20px;}input{width:80%;padding:10px;margin:10px 0;}button{padding:10px 20px;}</style>
    </head><body>
        <h2>ESP32-C3 Spotify Configuration</h2>
        <p>Latest Access Token:<br><small>)rawliteral" + (latestAccess.isEmpty() ? "None" : latestAccess) + R"rawliteral(</small></p>
        <p>Latest Refresh Token:<br><small>)rawliteral" + (latestRefresh.isEmpty() ? "None" : latestRefresh) + R"rawliteral(</small></p>
        <form action="/set-config" method="POST">
            <label>New Access Token:</label><br>
            <input type="text" name="atoken"><br>
            <label>New Refresh Token:</label><br>
            <input type="text" name="rtoken"><br>
            <label>WiFi SSID:</label><br>
            <input type="text" name="ssid" value=")rawliteral" + wifiSsid + R"rawliteral("><br>
            <label>WiFi Password:</label><br>
            <input type="password" name="pass" value=")rawliteral" + wifiPass + R"rawliteral("><br>
            <button type="submit">Save</button>
        </form>
    </body></html>
    )rawliteral";

    server.send(200, "text/html", html);
}

void handleSetConfig() {
    bool changed = false;

    if (server.hasArg("atoken") && server.hasArg("rtoken") &&
        !server.arg("atoken").isEmpty() && !server.arg("rtoken").isEmpty()) {
        save_token(server.arg("atoken"), server.arg("rtoken"));
        changed = true;
    }

    if (server.hasArg("ssid") && server.hasArg("pass")) {
        wifiSsid = server.arg("ssid");
        wifiPass = server.arg("pass");
        preferences.begin("spotify", false);
        preferences.putString("wifi_ssid", wifiSsid);
        preferences.putString("wifi_pass", wifiPass);
        preferences.end();
        changed = true;
    }

    if (changed) {
        server.send(200, "text/html", "<html><body><h2>Configuration Saved! Rebooting...</h2></body></html>");
        delay(1000);
        ESP.restart();
    } else {
        server.send(200, "text/html", "<html><body><h2>No changes.</h2><a href='/'>Back</a></body></html>");
    }
}

// ======================
// Track Handling
// ======================
void checkTrackLikeStatus() {
    if (currentTrackId.isEmpty()) { isTrackLiked = false; return; }
    String resp;
    int code = spotifyRequest("GET", "/me/tracks/contains?ids=" + currentTrackId, resp);
    if (code == 200) isTrackLiked = resp.indexOf("true") != -1;
    else isTrackLiked = false;
}

void fetchCurrentTrack() {
    String resp;
    int code = spotifyRequest("GET", SPOTIFY_CURRENTLY_PLAYING_URL, resp);
    if (code == 200) {
        StaticJsonDocument<2048> doc;
        if (deserializeJson(doc, resp) == DeserializationError::Ok && doc.containsKey("item")) {
            JsonObject item = doc["item"];
            String newTrackId = item["id"].as<String>();
            if (newTrackId != currentTrackId) { titleOffset = 0; artistOffset = 0; }
            songTitle = item["name"].as<String>();
            currentTrackId = newTrackId;
            artist = "";
            for (JsonObject a : item["artists"].as<JsonArray>()) {
                if (!artist.isEmpty()) artist += ", ";
                artist += a["name"].as<String>();
            }
            totalSeconds = item["duration_ms"].as<int>() / 1000;
            currentSeconds = doc["progress_ms"].as<int>() / 1000;
            isPlaying = doc["is_playing"] | false;
            isBackEnabled = true;
            isNextEnabled = true;
            checkTrackLikeStatus();
        }
    }
}

// ======================
// UI helpers
// ======================
void drawScrollingText(int y, const String& text, int offset) {
    int len = customFont.getLength(text.c_str());
    if (len <= SCREEN_WIDTH) customFont.print_noBackColor(0,y,text.c_str(),1);
    else {
        int adjusted = offset % (len + SCREEN_WIDTH);
        if (adjusted < len) customFont.print_noBackColor(-adjusted,y,text.c_str(),1);
        else customFont.print_noBackColor(len-adjusted,y,text.c_str(),1);
    }
}

String formatTime(int s) { 
    char buf[6]; 
    sprintf(buf,"%02d:%02d",s/60,s%60); 
    return String(buf); 
}

void drawUserInterface() {
    display.clearDisplay();
    customFont.set_font(jp2);
    drawScrollingText(0, songTitle, titleOffset);
    drawScrollingText(14, artist, artistOffset);
    customFont.print_noBackColor(0,32,formatTime(currentSeconds).c_str(),1);
    customFont.print_noBackColor(99,32,formatTime(totalSeconds).c_str(),1);

    int barX=30, barY=36, barW=63, barH=6;
    display.drawRect(barX,barY,barW,barH,SSD1306_WHITE);
    int filled = (totalSeconds>0)? map(currentSeconds,0,totalSeconds,0,barW-2):0;
    display.fillRect(barX+1,barY+1,filled,barH-2,SSD1306_WHITE);

    customFont.print_noBackColor(112,45,isTrackLiked?"O":"X",1);

    int baseY=45;
    customFont.print_noBackColor(5,baseY,isBackEnabled?"<":"<<",1);
    customFont.print_noBackColor(isPlaying?40:41,baseY,isPlaying?"||":">",1);
    customFont.print_noBackColor(75,baseY,isNextEnabled?">":">>",1);

    display.display();
}

// ======================
// Buttons
// ======================
void handleButtons() {
    unsigned long now=millis();
    if(digitalRead(BUTTON_PREV)==LOW && now-lastButtonPress[0]>DEBOUNCE_MS){
        String dummy; 
        spotifyRequest("POST","/me/player/previous",dummy); 
        fetchCurrentTrack(); 
        lastButtonPress[0]=now;
    }
    if(digitalRead(BUTTON_NEXT)==LOW && now-lastButtonPress[2]>DEBOUNCE_MS){
        String dummy; 
        spotifyRequest("POST","/me/player/next",dummy); 
        fetchCurrentTrack(); 
        lastButtonPress[2]=now;
    }
    if(digitalRead(BUTTON_LIKE)==LOW && now-lastButtonPress[3]>DEBOUNCE_MS){
        String dummy;
        if(!currentTrackId.isEmpty()){
            if(isTrackLiked) spotifyRequest("DELETE","/me/tracks?ids="+currentTrackId,dummy);
            else spotifyRequest("PUT","/me/tracks?ids="+currentTrackId,dummy);
            checkTrackLikeStatus();
        }
        lastButtonPress[3]=now;
    }
}

void handleToggleButton() {
    unsigned long now=millis();
    bool state=digitalRead(BUTTON_TOGGLE);
    if(lastToggleButtonState==HIGH && state==LOW && now-lastButtonPress[1]>DEBOUNCE_MS){
        lastButtonPress[1]=now;
        String dummy;
        if(isPlaying) spotifyRequest("PUT","/me/player/pause",dummy);
        else spotifyRequest("PUT","/me/player/play",dummy);
    }
    lastToggleButtonState=state;
}

// ======================
// Setup + Loop
// ======================
void setup() {
    Serial.begin(115200);
    Wire.begin(20,21);
    if(!display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS)){ for(;;); }
    customFont.set_font(jp2);
    display.clearDisplay(); 
    customFont.print_noBackColor(0,0,"Connecting WiFi...",1); 
    display.display();

    preferences.begin("spotify", false);
    wifiSsid = preferences.getString("wifi_ssid","Wifi_here");
    wifiPass = preferences.getString("wifi_pass","Pass_here");
    preferences.end();

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(wifiSsid.c_str(),wifiPass.c_str());
    unsigned long start=millis();
    while(WiFi.status()!=WL_CONNECTED && millis()-start<10000){ delay(500); }

    display.clearDisplay();
    if(WiFi.status()==WL_CONNECTED){
        String ip=WiFi.localIP().toString();
        customFont.print_noBackColor(0,0,"STA IP:",1);
        customFont.print_noBackColor(0,14,ip.c_str(),1);
    } else {
        customFont.print_noBackColor(0,0,"WiFi Failed",1);
        customFont.print_noBackColor(0,14,"Use AP Mode",1);
    }
    customFont.print_noBackColor(0,28,"AP:192.168.4.1",1);
    display.display(); 
    delay(2000);

    WiFi.softAP(AP_SSID,AP_PASS);
    server.on("/",handleRoot);
    server.on("/set-config",HTTP_POST,handleSetConfig);
    server.begin();

    pinMode(BUTTON_PREV,INPUT_PULLUP);
    pinMode(BUTTON_TOGGLE,INPUT_PULLUP);
    pinMode(BUTTON_NEXT,INPUT_PULLUP);
    pinMode(BUTTON_LIKE,INPUT_PULLUP);

    Serial.println("[SETUP] Ready. Visit http://192.168.4.1");
}

void loop() {
    server.handleClient();
    handleButtons();
    handleToggleButton();
    unsigned long now=millis();
    if(now-lastFetchTime>FETCH_INTERVAL_MS){ 
        fetchCurrentTrack(); 
        lastFetchTime=now; 
    }
    if(now-lastScrollTime>SCROLL_INTERVAL_MS){ 
        titleOffset++; 
        artistOffset++; 
        drawUserInterface(); 
        lastScrollTime=now; 
    }
}
