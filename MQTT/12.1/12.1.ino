#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ====== CONFIG ====== */
const char* device_name = "LED_MQTT";

const char* ssid = "CAU VI";
const char* password = "0968978073";

// IP Home Assistant
const char* mqtt_server = "192.168.1.146";
const int mqtt_port = 1883;

const char* mqtt_user = "wind1";
const char* mqtt_pass = "1234";

const char* student_id = "10124346";
const char* mqtt_topic = "iot/lab1/10124346/sensor";

/* ==================== */

WiFiClient espClient;
PubSubClient client(espClient);

// Biến hỗ trợ gửi dữ liệu định kỳ không dùng delay()
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    String clientId = String(device_name) + "_" + String(student_id);
    Serial.print("Attempting MQTT connection... ");

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" -> try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Đảm bảo duy trì kết nối MQTT liên tục
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Gửi dữ liệu định kỳ mỗi 5000ms (5 giây) mà không làm nghẽn mạch
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    // Khởi tạo JSON (Phiên bản tương thích ArduinoJson v6 và v7)
    #if ARDUINOJSON_VERSION_MAJOR >= 7
      JsonDocument doc;
    #else
      StaticJsonDocument<200> doc;
    #endif

    doc["temp"] = random(250, 350) / 10.0;
    doc["hum"] = random(400, 800) / 10.0;

    char payload[128];
    serializeJson(doc, payload);
    
    // Gửi và in ra màn hình Serial để debug
    if (client.publish(mqtt_topic, payload)) {
      Serial.print("Published: ");
      Serial.println(payload);
    } else {
      Serial.println("Publish failed!");
    }
  }
}
