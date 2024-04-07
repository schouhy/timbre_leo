#include "Arduino.h"
#include "configuration.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"

#include "UniversalTelegramBot.h"

#define CHAT_ID "226959124"
#define RING_PIN 23 

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
    Serial.println();
}


void loop() {

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
