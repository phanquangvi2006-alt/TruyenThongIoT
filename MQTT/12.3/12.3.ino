#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h> // Thư viện điều khiển LED RGB NeoPixel của ESP32-S3

/* ===== CONFIG ===== */
const char* ssid = "CAU VI";
const char* password = "0968978073";

const char* mqtt_server = "192.168.1.146";
const int mqtt_port = 1883;

/* MQTT account (tạo trong Mosquitto HA) */
const char* mqtt_user = "wind1";
const char* mqtt_pass = "1234";

const char* student_id = "sv01";
/* ================== */

WiFiClient espClient;
PubSubClient client(espClient);

/* ===== TOPIC (tự động theo student_id) ===== */
String topic_cmd;
String topic_state;
String topic_note;
String topic_lwt;
/* =========================================== */

/* ===== CẤU HÌNH LED RGB ONBOARD ESP32-S3 ===== */
// Mạch ESP32-S3 thông thường sử dụng chân 48, một số bản thu nhỏ dùng chân 38 hoặc 8.
#define RGB_PIN        48  
#define NUMPIXELS      1   

Adafruit_NeoPixel rgbLed(NUMPIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);
bool ledState = false;

/* ===== LED CONTROL ===== */
void applyLed(bool on) {
  ledState = on;
  if (on) {
    // Bật LED màu xanh lá (Green = 150), bạn có thể đổi thành màu khác tùy ý
    rgbLed.setPixelColor(0, rgbLed.Color(0, 150, 0));
  } else {
    // Tắt LED (Màu đen)
    rgbLed.setPixelColor(0, rgbLed.Color(0, 0, 0));
  }
  rgbLed.show(); // Cập nhật trạng thái đèn vật lý
}

void publishStateRetained() {
  client.publish(topic_state.c_str(), ledState ? "ON" : "OFF", true);
}

/* ===== WIFI ===== */
void setup_wifi() {
  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

/* ===== MQTT CALLBACK ===== */
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();
  Serial.print("MQTT [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(msg);

  if (String(topic) == topic_cmd) {
    if (msg == "ON") applyLed(true);
    else if (msg == "OFF") applyLed(false);
    else if (msg == "TOGGLE") applyLed(!ledState);
    publishStateRetained();
  }
}

/* ===== MQTT RECONNECT ===== */
void reconnect() {
  while (!client.connected()) {
    // Cập nhật tên thiết bị cho đúng bản chất phần cứng ESP32-S3
    String clientId = "ESP32S3-Lab3-";
    clientId += student_id;
    Serial.print("Connecting MQTT... ");
    
    if (client.connect(
      clientId.c_str(),
      mqtt_user,
      mqtt_pass,
      topic_lwt.c_str(),
      0,
      true,
      "OFFLINE"
    )) {
      Serial.println("connected");
      client.subscribe(topic_cmd.c_str());
      Serial.print("Subscribed ");
      Serial.println(topic_cmd);
      
      client.publish(topic_note.c_str(), 
                     "ESP32-S3 BOOT / RECONNECT (note not retained)", 
                     false);

      client.publish(topic_lwt.c_str(), "ONLINE", true);
      publishStateRetained();
    } else {
      Serial.print("failed rc=");
      Serial.println(client.state());
      delay(1500);
    }
  }
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  // Khởi tạo hệ thống đèn LED RGB của ESP32-S3
  rgbLed.begin();
  applyLed(false);

  /* Build topics */
  topic_cmd = "iot/lab3/" + String(student_id) + "/cmd";
  topic_state = "iot/lab3/" + String(student_id) + "/state";
  topic_note = "iot/lab3/" + String(student_id) + "/note";
  topic_lwt = "iot/lab3/" + String(student_id) + "/lwt";
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(60);
}

/* ===== LOOP ===== */
void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
