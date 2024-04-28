///////////////////////////////////////
// INCLUDE
///////////////////////////////////////

#include <WiFi.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>

///////////////////////////////////////
// DEFINE
///////////////////////////////////////

#define SERIAL_DEBUG_BAUD 115200
#define WIFI_SSID "wlan_2.4G"
#define WIFI_PASSWORD "0891560526"
#define THINGSBOARD_SERVER "192.168.1.134"
#define THINGSBOARD_PORT 1883
#define DEVICE_ACCESS_TOKEN "1pcb2vcwfyy9w2l31ebj"
#define MAX_MESSAGE_SIZE 1024

///////////////////////////////////////
// GLOBAL VARIABLES
///////////////////////////////////////

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

// for upload telemetry
unsigned long uploadTimer = 0;
unsigned long uploadInterval = 2000;

///////////////////////////////////////
// FUNCIONS
///////////////////////////////////////

void InitWiFi() {
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

const bool reconnect() {
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  InitWiFi();
  return true;
}


///////////////////////////////////////
// MAIN
///////////////////////////////////////

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  InitWiFi();
}

void loop() {

  // 1. Check WiFi Connection
  if (!reconnect()) {
    delay(1000);
    return;
  }

  // 2. Connect to ThingsBoard
  if (!tb.connected()) {
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(DEVICE_ACCESS_TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, DEVICE_ACCESS_TOKEN, THINGSBOARD_PORT)) {
      Serial.println("Failed to connect");
      return;
    }
    Serial.println("Connect success");
  }

  // 3. upload telemetry data
  unsigned long ts = millis();
  if (ts - uploadTimer > uploadInterval) {
    uploadTimer = ts;

    // upload
    Serial.println("upload...");
    tb.sendTelemetryData("temperature", random(20, 30));
    tb.sendTelemetryData("humidity", random(3, 8) * 10);
  }
}


