#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

#include <Arduino.h>
#include <WiFi.h>

#include <esp_wifi.h>
#include <esp_event.h>

// GPIO Setup
const int LED = 32; // On while scanning

// WiFi Setup (Target AP)
const char *ssid = "ESP32-AP";
const char *password = "aaaabbbb";
const uint8_t bssid[] = {0xB8, 0xDD, 0x71, 0xE5, 0xC2, 0x63};
const int channel = 9;

// Pretty print encryption type
String encType2String(wifi_auth_mode_t authMode) {
    switch(authMode) {
        case WIFI_AUTH_OPEN:
            return "Open";
        case WIFI_AUTH_WEP:
            return "WEP";
        case WIFI_AUTH_WPA_PSK:
            return "WPA_PSK";
        case WIFI_AUTH_WPA2_PSK:
            return "WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA_WPA2_PSK";
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2_ENTERPRISE";
        case WIFI_AUTH_MAX:
            return "MAX";
        default:
            return "Unknown";
    }    
}

// Scan for WiFi networks and print results
void wifiScan() {
    const int outbuf_len = 77;
    char buffer[outbuf_len];

    // Set WiFi to station
    WiFi.mode(WIFI_STA);

    // WiFi.scanNetworks will return the number of networks found
    Serial.println("Scanning for WiFi networks... 🔍");
    digitalWrite(LED, HIGH);
    int n = WiFi.scanNetworks(false, true);
    Serial.println("Scan done! ✅");
    digitalWrite(LED, LOW);

    // No networks found
    if (n == 0) {
        Serial.println("No networks found 😢");
        return;
    }

    // Print number of networks found and table header
    Serial.println(String(n) + " networks found:");
    Serial.println("ID  SSID                              MAC                CH  ENC");
    Serial.println("-------------------------------------------------------------------------");
    for (int i = 0; i < n; ++i) {
        snprintf(buffer, outbuf_len, "%2d  %-32s  %s  %2d  %s",
            i + 1,
            WiFi.SSID(i).c_str(),
            WiFi.BSSIDstr(i).c_str(),
            WiFi.channel(i),
            encType2String(WiFi.encryptionType(i)).c_str());
        Serial.println(buffer);
    }
    Serial.println();
}

// Create an AP with the same SSID and MAC address as the target AP
void createAP() {
    // Set WiFi to access point mode 
    WiFi.mode(WIFI_AP);

    Serial.println("Creating AP... 📡");
    digitalWrite(LED, LOW);
    
    // Set MAC address to target AP's MAC address
    esp_wifi_set_mac(WIFI_IF_AP, &bssid[0]);
    if (!WiFi.softAP(ssid, password, channel)) {
        Serial.println("AP creation failed ❌");
        return;
    }
    
    Serial.println("AP created! ✅");
    Serial.println("  SSID : " + String(ssid));
    Serial.println("  MAC  : " + String(WiFi.softAPmacAddress()));
    Serial.println("  CH   : " + String(channel));
    Serial.println("  ENC  : TODO"); 
    // TODO: Encryption type
    Serial.println();

    digitalWrite(LED, HIGH);    
}

// WiFi event handler
void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case 12:
            Serial.printf("📶[WiFi-event] %d: Client connected! 📱\n", event);
            break;
        case 13:
            Serial.printf("📶[WiFi-event] %d: Client disconnected! 📴\n", event);
            break;
        case 14:
            Serial.printf("📶[WiFi-event] %d: IP assigned! ☑️\n", event);
            break;
        default:
            Serial.printf("📶[WiFi-event] event %d\n", event);
            break;
    }
}

void setup() {
    // Disconnect from any WiFi networks if connected
    WiFi.disconnect();

    // Initialize LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    // Wait for a message on the serial port
    Serial.begin(115200);
    Serial.println("Press any key to start... ⌨️");
    while (!Serial.available()) {
        ;
    }
    Serial.println("Starting...");

    // Start an access point
    createAP();
    WiFi.onEvent(WiFiEvent);

    delay(100);
}

void loop() {

}
