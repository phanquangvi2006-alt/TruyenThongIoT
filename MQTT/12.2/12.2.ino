#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h> // Thư viện điều khiển LED RGB Onboard (NeoPixel)

/* ===== CONFIG ===== */
const char* ssid = "CAU VI";
const char* password = "0968978073";
const char* mqtt_server = "192.168.1.146";
const int mqtt_port = 1883;

/* MQTT ACCOUNT (tạo trong Mosquitto HA) */
const char* mqtt_user = "wind1";
const char* mqtt_pass = "1234";
const char* student_id = "sv01";

/* MQTT TOPICS */
const char* topic_cmd = "iot/lab2/sv01/cmd";
const char* topic_state = "iot/lab2/sv01/state";
/* ================== */

WiFiClient espClient;
PubSubClient client(espClient);

#define RGB_PIN        48 
#define NUMPIXELS      1 

Adafruit_NeoPixel rgbLed(NUMPIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);
bool ledState = false;

/* ================== WIFI ================== */
void setup_wifi() {
  Serial.print("Connecting to WiFi ");
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

/* ================== MQTT CALLBACK ================== */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (message == "ON") {
    ledState = true;
    // Bật đèn màu TRẮNG (Độ sáng tối đa: Đỏ 255, Xanh lá 255, Xanh dương 255)
    rgbLed.setPixelColor(0, rgbLed.Color(255, 255, 255)); 
    rgbLed.show();
  }
  else if (message == "OFF") {
    ledState = false;
    // Tắt đèn (Xóa màu về 0)
    rgbLed.setPixelColor(0, rgbLed.Color(0, 0, 0)); 
    rgbLed.show();
  }
  // Mở rộng thêm: Nếu bạn gửi mã màu dạng "R,G,B" (ví dụ: "255,0,0" là màu đỏ)
  else if (message.indexOf(",") > 0) {
    ledState = true;
    int firstComma = message.indexOf(',');
    int secondComma = message.indexOf(',', firstComma + 1);
    
    int r = message.substring(0, firstComma).toInt();
    int g = message.substring(firstComma + 1, secondComma).toInt();
    int b = message.substring(secondComma + 1).toInt();
    
    rgbLed.setPixelColor(0, rgbLed.Color(r, g, b));
    rgbLed.show();
  }

  /* Publish trạng thái LED về lại broker */
  client.publish(topic_state, ledState ? "ON" : "OFF");
}

/* ================== MQTT RECONNECT ================== */
void reconnect() {
  while (!client.connected()) {
    // Sửa ID thiết bị thành ESP32 cho đúng bản chất phần cứng
    String clientId = "ESP32-Lab2-";
    clientId += student_id;
    Serial.print("Connecting to MQTT... ");
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(topic_cmd);
      Serial.print("Subscribed to ");
      Serial.println(topic_cmd);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retry in 2s");
      delay(2000);
    }
  }
}

/* ================== SETUP ================== */
void setup() {
  Serial.begin(115200);
  
  // Khởi tạo hệ thống LED RGB
  rgbLed.begin();
  rgbLed.setPixelColor(0, rgbLed.Color(0, 0, 0)); // Tắt đèn lúc khởi động
  rgbLed.show();

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(60);
}

/* ================== LOOP ================== */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
