#include <WiFi.h>

#define SERIAL_DEBUG_BAUD 115200
#define WIFI_SSID "wlan_2.4G"
#define WIFI_PASSWORD "0891560526"

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



void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  InitWiFi();
}

void loop() {
  if (!reconnect()) {
    delay(1000);
    return;
  }
}


