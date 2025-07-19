#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

// === CONFIGURATION ===
const char* ssid = "Rasp_Home";
const char* password = "Password";

const char* mqtt_server = "192.168.4.1";
const int mqtt_port = 8883;

// OTA
const char* currentFirmwareVersion = "v1.0.0";
const String versionCheckURL = "http://192.168.4.1/version.txt";
const String firmwareBinaryURL = "http://192.168.4.1/switch.bin";

// Pins
#define RELAY_PIN 5   // D5
#define BUTTON_PIN 12   // D6

// MQTT Topics
#define TOPIC_SWITCH "home/switch1"
#define TOPIC_STATUS "home/switch1/status"

WiFiClientSecure espClient;
PubSubClient client(espClient);

bool relayState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool lastButtonState = HIGH;

void setup_wifi() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("SmartSwitchClient", "B15", "buddy")) {
      Serial.println("connected ✅");
      client.subscribe(TOPIC_SWITCH);
    } else {
      Serial.print("Failed. Code: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void publishStatus() {
  String statusMsg = relayState ? "ON" : "OFF";
  client.publish(TOPIC_STATUS, statusMsg.c_str(), true);
}

void toggleRelay(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  Serial.println(state ? "🔌 Relay ON" : "🔌 Relay OFF");
  publishStatus();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  message.trim();
  
  Serial.printf("📩 MQTT [%s]: %s\n", topic, message.c_str());

  if (String(topic) == TOPIC_SWITCH) {
    if (message == "ON") toggleRelay(true);
    else if (message == "OFF") toggleRelay(false);
  }
}

void checkAndUpdateFirmware() {
  Serial.println("🔍 Checking for firmware update...");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, versionCheckURL);

    int httpCode = http.GET();
    if (httpCode == 200) {
      String newVersion = http.getString();
      newVersion.trim();
      Serial.println("📄 Server version: " + newVersion);

      if (newVersion != currentFirmwareVersion) {
        Serial.println("⬆️ Update available. Starting OTA...");
        t_httpUpdate_return ret = ESPhttpUpdate.update(client, firmwareBinaryURL);

        switch (ret) {
          case HTTP_UPDATE_FAILED:
            Serial.printf("❌ OTA Failed (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
          case HTTP_UPDATE_NO_UPDATES:
            Serial.println("ℹ️ No updates found.");
            break;
          case HTTP_UPDATE_OK:
            Serial.println("✅ OTA Success. Rebooting...");
            break;
        }
      } else {
        Serial.println("✅ Already up-to-date.");
      }
    } else {
      Serial.printf("❌ Version check failed. Code: %d\n", httpCode);
    }
    http.end();
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  toggleRelay(false);  // Default OFF

  setup_wifi();
  checkAndUpdateFirmware();

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Manual toggle via button
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = reading;
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW) {
      toggleRelay(!relayState);
      delay(300);  // Simple debounce
    }
  }
}
