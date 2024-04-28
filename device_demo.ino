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
// for update client attribute
unsigned long updateTimer = 0;
unsigned long updateInterval = 8000;

// List of shared attributes for subscribing to their updates
const std::array<const char *, 2> SHARED_ATTRIBUTES_LIST = {
  "uploadIntervalMs",
  "updateIntervalMs"
};

void processSharedAttributes(const Shared_Attribute_Data &data);
RPC_Response setUploadMode(const RPC_Data &data);
RPC_Response doReset(const RPC_Data &data);

const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());

const std::array<RPC_Callback, 2> rpc_callbacks = {
  RPC_Callback{"restart", doReset },
  RPC_Callback{"setUploadMode", setUploadMode },
};

// FIX ISSUE RPC NOT RESPONSE
DynamicJsonDocument doc(1024);

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

// CALLBACK
void processSharedAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), "uploadIntervalMs") == 0) {
      const uint32_t v = it->value().as<uint32_t>();
      if (v < 500 || v > 60000) {
        continue;
      }
      uploadInterval = v;
      Serial.print("set uploadInterval to ");
      Serial.println(v);
    } else if (strcmp(it->key().c_str(), "updateIntervalMs") == 0) {
      const uint32_t v = it->value().as<uint32_t>();
      if (v < 2000 || v > 60000) {
        continue;
      }
      updateInterval = v;
      Serial.print("set updateInterval to ");
      Serial.println(v);
    }
  }
  // attributesChanged = true;
}

RPC_Response doReset(const RPC_Data &data) {
  Serial.println("Received reset RPC method");

  // Process data
  int delayBeforeReset = data;

  Serial.print("delay before reset");
  Serial.println(delayBeforeReset);

  if (delayBeforeReset < 0 || delayBeforeReset > 1000) {
    return RPC_Response("error", "delay should be 0 to 1000 ms");
  }
  
  doc.clear();
  doc["status"] = true;
  return RPC_Response(doc);
}


RPC_Response setUploadMode(const RPC_Data &data) {
  Serial.println("Received setUploadMode RPC method");

  // Process data
  int newMode = data;

  Serial.print("setUploadMode");
  Serial.println(newMode);

  return RPC_Response("newMode", newMode);
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

    if(tb.RPC_Subscribe(rpc_callbacks.cbegin(), rpc_callbacks.cend())) {
      Serial.println("subscribe rpc ok");
    }
    tb.Shared_Attributes_Subscribe(attributes_callback);
    tb.Shared_Attributes_Request(attribute_shared_request_callback);
  }

  unsigned long ts = millis();

  // 3. upload telemetry data
  if (ts - uploadTimer > uploadInterval) {
    uploadTimer = ts;

    // upload
    Serial.println("upload...");
    // tb.sendTelemetryData("temperature", random(20, 30));
    // tb.sendTelemetryData("humidity", random(3, 8) * 10);
    Telemetry data[] = {
      Telemetry("temperature", random(20, 30)),
      Telemetry("humidity",  random(3, 8) * 10)
    };
    tb.sendTelemetry(data, 2);
  }

  // 4. update client attributes
  if (ts - updateTimer > updateInterval) {
    updateTimer = ts;
    int rssi = WiFi.RSSI();
    int channel = WiFi.channel();
    char bssid[64];
    char localIp[64];
    char ssid[64];
    strcpy(bssid, WiFi.BSSIDstr().c_str());
    strcpy(localIp, WiFi.localIP().toString().c_str());
    strcpy(ssid, WiFi.SSID().c_str());

    Serial.print("update rssi=");
    Serial.print(rssi);
    Serial.print(" channel=");
    Serial.print(channel);
    Serial.print(" bssid=");
    Serial.print(bssid);
    Serial.print(" localIp=");
    Serial.print(localIp);
    Serial.print(" ssid=");
    Serial.println(ssid);
    
    // tb.sendAttributeData("rssi", rssi);
    // tb.sendAttributeData("channel", channel);
    // tb.sendAttributeData("bssid", bssid);
    // tb.sendAttributeData("localIp", localIp);
    // tb.sendAttributeData("ssid", ssid);
    
    // Attribute data[] = {
    //   Attribute("rssi", WiFi.RSSI()),
    //   Attribute("channel",  WiFi.channel()),
    //   Attribute("bssid",  WiFi.BSSIDstr().c_str()),
    //   Attribute("localIp",  WiFi.localIP().toString().c_str()),
    //   Attribute("ssid",  WiFi.SSID().c_str()),
    // };
    Attribute data[] = {
      Attribute("rssi", rssi),
      Attribute("channel", channel),
      Attribute("bssid", bssid),
      Attribute("localIp", localIp),
      Attribute("ssid", ssid),
    };
    tb.sendAttributes(data, 5);
  }

  tb.loop();
}


