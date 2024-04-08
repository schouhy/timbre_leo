#include "Arduino.h"
#include <FS.h>
#include "configuration.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"

#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#include "UniversalTelegramBot.h"

#define RING_PIN 23 
#define CONFIG_PIN 21 

#define RW_MODE false
#define RO_MODE true
#define WIFI_CONFIG_NAMESPACE "WiFiConfig"
Preferences wifi_config;

// Configuration mode AP settings
AsyncWebServer server(80);
const char* ap_ssid = "LmDTimbreManager"; 
const char* ap_password = ""; 
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Configuration mode page
const char* htmlContent = R"(
<!DOCTYPE html>
<html>
<head>
  <title>LmD Tech ®</title>
</head>
<body>
  <h1>Hello from LmD timbre manager! &#x1F60E;</h1>
  <p>You have entered in configuration mode.</p>
</body>
</html>
<h1>Change SSID configuration</h1>
<form action='/submit' method='post'>
<label for='ssid'>New SSID:</label>
<input type='text' id='ssid' name='ssid'><br><br>
<label for='password'>SSID Password:</label>
<input type='text' id='password' name='password'><br><br>
<input type='submit' value='Submit'>
</form></body></html>
)";

bool ringed = false;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime = 0;

// ########################################
// ##### Configuration mode methods #######
// ########################################

void config_mode_setup() {
    Serial.println("Config mode");
    wifi_config.end();
    wifi_config.begin(WIFI_CONFIG_NAMESPACE, RW_MODE);

    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress ip = WiFi.softAPIP();
    Serial.print("Configuration site at ip: ");
    Serial.println(ip);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(200, "text/html", htmlContent);
    });
    server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("ssid", true)) {
            wifi_config.putString("SSID", request->getParam("ssid", true)->value());
        }
        if (request->hasParam("password", true)) {
            wifi_config.putString("SSIDPassword", request->getParam("password", true)->value());
        }
        request->send(200, "text/plain", "Data received and stored!");
    });

    server.begin();
}

void config_mode_loop(void* params) {
    while(true) {
        delay(1000);
    }
}

// ########################################
// ######## Normal mode methods ###########
// ########################################

void handleRingPress() {
    ringed = true;
}

void normal_mode_setup() {
    Serial.println("Normal mode");
    attachInterrupt(RING_PIN, handleRingPress, FALLING);

    Serial.print("Connecting to Wifi SSID ");
    Serial.print(wifi_config.getString("SSID"));
    Serial.print(" ");
    WiFi.begin(wifi_config.getString("SSID"), wifi_config.getString("SSIDPassword"));
    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    Serial.print("Certificate loaded");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    delay(100);
    Serial.print("ok!");
}

void sendHTML(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println(htmlContent); // Send the HTML content
}


void normal_mode_loop(void* params) {
    while(true) {
        if (millis() - bot_lasttime > BOT_MTBS) {
            int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

            if (numNewMessages) {
                for(int i = 0; i < numNewMessages; i++) {
                    telegramMessage &msg = bot.messages[i];
                    Serial.println("Received " + msg.text);
                    Serial.println("chat id " + msg.chat_id);
                }
            }

            bot_lasttime = millis();
        }
        delay(10);
        if (ringed) {
            bot.sendMessage(CHAT_ID, "Timbre!", "Markdown");
            delay(500);
            ringed = false;
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(RING_PIN, INPUT_PULLUP);
    pinMode(CONFIG_PIN, INPUT_PULLUP);
    wifi_config.begin(WIFI_CONFIG_NAMESPACE, RO_MODE);
    delay(100);

    if(digitalRead(CONFIG_PIN) == LOW) {
        config_mode_setup();
        xTaskCreate(config_mode_loop, "config_mode", 1024 * 8, nullptr, 2, nullptr);
    } else {
        normal_mode_setup();
        xTaskCreate(normal_mode_loop, "normal_mode", 1024 * 8, nullptr, 2, nullptr);
    }

    // Delete default loop task
    vTaskDelete(nullptr);
}

void loop() {
}
