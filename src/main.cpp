#include "Arduino.h"
#include <FS.h>
#include "configuration.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"

#include <ESPAsyncWebServer.h>

#include "UniversalTelegramBot.h"

#define CHAT_ID "226959124"
#define RING_PIN 23 

bool CONFIGURATION_MODE_AP = true; 
const char* ap_ssid = "your_SSID"; 
const char* ap_password = "your_PASSWORD"; 
AsyncWebServer server(80);

IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

String mySSID;
String mySSIDPassword;
const char* htmlContent = R"(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Web Server</title>
</head>
<body>
  <h1>Hello from ESP32!</h1>
  <p>This is a sample webpage served by ESP32.</p>
</body>
</html>
<h1>ESP32 Web Server</h1>
<form action='/submit' method='post'>
<label for='ssid'>New SSID:</label>
<input type='text' id='ssid' name='ssid'><br><br>
<label for='password'>SSID Password:</label>
<input type='text' id='password' name='password'><br><br>
<input type='submit' value='Submit'>
</form></body></html>
)";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

unsigned long bot_lasttime = 0;
bool ringed = false;

void handleRingPress() {
    ringed = true;
}

void setup() {
    Serial.begin(115200);
    pinMode(RING_PIN, INPUT_PULLUP);
    delay(100);

    if(CONFIGURATION_MODE_AP) {

        WiFi.softAPConfig(local_IP, gateway, subnet);
        WiFi.softAP(ap_ssid, ap_password);
        IPAddress ip = WiFi.softAPIP();
        Serial.print("Access Point IP address: ");
        Serial.println(ip);
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(200, "text/html", htmlContent);
        });
        server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request){
            if (request->hasParam("ssid", true)) {
                mySSID = request->getParam("ssid", true)->value();
            }
            if (request->hasParam("password", true)) {
                mySSIDPassword = request->getParam("password", true)->value();
            }
            request->send(200, "text/plain", "Data received and stored!");
        });

        server.begin();
    } else {
        attachInterrupt(RING_PIN, handleRingPress, FALLING);

        Serial.print("Connecting to Wifi SSID ");
        Serial.print(WIFI_SSID);
        Serial.print(" ");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
        Serial.print("Certificate loaded");
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(500);
        }
        delay(100); Serial.print("conectado");
    }
}

void sendHTML(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println(htmlContent); // Send the HTML content
}

void loop() {
    if(CONFIGURATION_MODE_AP) {
        delay(1000);
        Serial.println(mySSID + " " + mySSIDPassword);
    } else {
        //    if (millis() - bot_lasttime > BOT_MTBS) {
        //        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        //
        //        if (numNewMessages) {
        //              for(int i = 0; i < numNewMessages; i++) {
        //                    telegramMessage &msg = bot.messages[i];
        //                    Serial.println("Received " + msg.text);
        //                    Serial.println("chat id " + msg.chat_id);
        //              }
        //        }
        //      
        //        bot_lasttime = millis();
        //  }
        delay(10);
        if (ringed) {
            bot.sendMessage(CHAT_ID, "timbre", "Markdown");
            delay(500);
            ringed = false;
        }

    }

}
