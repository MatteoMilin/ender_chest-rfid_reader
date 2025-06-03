#include <Arduino.h>

#include <WiFi.h>
#include <map>
#include <HTTPClient.h>

#include <LittleFS.h>
#include <FS.h>
#include <SPI.h>
#include <MFRC522.h>

#define DEBUG  // Comment this line to disable debug messages

const char* ssid = "ENDER CHEST";
const char* serverURL = "http://192.168.4.1/uid";

#define RST_PIN  4
#define SS_PIN   5
#define SCK_PIN  18  // or 25
#define MISO_PIN 12
#define MOSI_PIN 13

#define CARD1 "108e821" // Carte Corentin
#define CARD2 "73f4f16" // Carte Vierge
#define CARD3 "70f8fa21" // Carte Matteo

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create instance
int state_scan = 0;

void check_scanned_card(String uid)
{
    if (uid == "")
        return;
    if (uid == CARD1 && state_scan == 0) {
        state_scan = 1;
        return;
    }
    if (uid == CARD2 && state_scan == 1) {
        state_scan = 2;
        return;
    }
    if (uid == CARD3 && state_scan == 2) {
        state_scan = 3;
        return;
    }
    state_scan = 0;
}

void setup() {
    Serial.begin(115200);
    #ifdef DEBUG // Wait for serial to connect
        while (!Serial.available()) delay(50);
    #endif
    Serial.println("Démarrage ESP32...");
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    mfrc522.PCD_Init();
    WiFi.begin(ssid);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\nConnected to AP");
    Serial.println("Finished loading");
}

void loop() {
    String uid = "";
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    Serial.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverURL);
        http.addHeader("Content-Type", "text/plain");
        Serial.println(uid);
        check_scanned_card(uid);
        if (state_scan == 3) {
            int code = http.POST("true");
            Serial.print("POST response code: ");
            Serial.println(code);
            state_scan = 0;
        }
        uid = "";
        http.end();
    }
    Serial.println();
    mfrc522.PICC_HaltA();
}