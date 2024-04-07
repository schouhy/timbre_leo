#include "Arduino.h"
#include "configuration.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"

#include "UniversalTelegramBot.h"

#define CHAT_ID "226959124"
#define RING_PIN 23 

bool CONFIGURATION_MODE_AP = true;

const char* ap_ssid = "your_SSID"; 
const char* ap_password = "your_PASSWORD"; 
WiFiServer server(80);

IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

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
        WiFiClient client = server.available();
        if(client) {
            Serial.println("Client connected.");
            while(client.connected()) {
                if(client.available()) {
                    String request = client.readStringUntil('\r');
                    Serial.println(request);
                    sendHTML(client);
                    break;
                }
            }
            delay(1);
            client.stop();
            Serial.println("Client disconnected.");
        }

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
