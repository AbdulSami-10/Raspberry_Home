#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

// Current firmware version
const char* currentFirmwareVersion = "v1.0.1";
const String versionCheckURL = "http://192.168.4.1/version.txt";  // version.txt on Pi
const char* firmwareBinaryURL = "http://192.168.4.1/firmware.bin"; // binary on Pi

// Wi-Fi credentials
const char* ssid = "Rasp_Home";
const char* password = "Password";

// MQTT broker
const char* mqtt_server = "192.168.4.1";
const int mqtt_port = 8883;

// DHT11 Sensor Setup
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQTT client
WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

void setup_wifi() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected. IP address:");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", "B15", "buddy")) {
      Serial.println("connected ✅");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" — retrying in 5 seconds");
      delay(5000);
    }
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
      Serial.print("📄 Server firmware version: ");
      Serial.println(newVersion);

      if (newVersion != currentFirmwareVersion) {
        Serial.println("⬆️ New version available. Starting OTA update...");
        WiFiClient client;
        t_httpUpdate_return result = ESPhttpUpdate.update(client, firmwareBinaryURL);

        switch (result) {
          case HTTP_UPDATE_FAILED:
            Serial.printf("❌ OTA failed. Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
          case HTTP_UPDATE_NO_UPDATES:
            Serial.println("ℹ️ No updates available.");
            break;
          case HTTP_UPDATE_OK:
            Serial.println("✅ OTA Update Successful! Rebooting...");
            break;
        }

      } else {
        Serial.println("✅ Firmware is up-to-date.");
      }
    } else {
      Serial.printf("❌ Failed to fetch version.txt. HTTP code: %d\n", httpCode);
    }

    http.end();
  }
}

void setup() {
  setup_wifi();
  checkAndUpdateFirmware();
  dht.begin();

  // TLS (skip cert validation for now)
  espClient.setInsecure();
  espClient.setBufferSizes(512, 512);
  client.setServer(mqtt_server, mqtt_port);

  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("❌ Failed to read from DHT sensor!");
      return;
    }

    char tempStr[8], humStr[8];
    dtostrf(temp, 1, 2, tempStr);
    dtostrf(hum, 1, 2, humStr);

    String tempMessage = "Temp: " + String(tempStr) + " C";
    String humMessage  = "Humidity: " + String(humStr) + " %";

    client.publish("sensor/temp", tempMessage.c_str());
    client.publish("sensor/hum", humMessage.c_str());

    Serial.print("🌡️ Temp: ");
    Serial.print(tempStr);
    Serial.print(" °C | 💧 Humidity: ");
    Serial.print(humStr);
    Serial.println(" %");
  }
}
