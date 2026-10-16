#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h> // Thư viện điều khiển LED RGB WS2812

// ====== CẤU HÌNH HỆ THỐNG ====== //
const char* device_name   = "ESP32_S3";
const char* ssid          = "wind";
const char* password      = "windphan59";

// Thông tin MQTT Broker
const char* mqtt_server   = "10.195.239.135";
const int mqtt_port       = 1883;
const char* mqtt_user     = "wind1";
const char* mqtt_pass     = "1234";

// Cấu hình Topic
const char* student_id    = "10124346"; 
const char* topic_sensor  = "iot/lab1/10124346/sensor";
const char* topic_led     = "iot/lab1/10124346/led"; // Topic điều khiển LED

// Cấu hình LED RGB WS2812
#define RGB_PIN        48  // Chân dữ liệu LED RGB
#define NUM_LEDS       1   // Số lượng LED onboard (thường là 1)

// Khởi tạo đối tượng LED RGB
Adafruit_NeoPixel rgbLed(NUM_LEDS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// ================================ //

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

// Hàm hỗ trợ đổi màu LED đơn giản
void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  rgbLed.setPixelColor(0, rgbLed.Color(r, g, b));
  rgbLed.show();
}

// Hàm Callback xử lý tín hiệu MQTT
// Hàm Callback xử lý tín hiệu MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Tin nhắn đến [");
  Serial.print(topic);
  Serial.print("]: ");

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Kiểm tra nếu đúng topic điều khiển LED
  if (String(topic) == topic_led) {
    
    // 1. Nếu tin nhắn chứa dấu phẩy (dạng "R,G,B", ví dụ: "255,0,128")
    if (message.indexOf(',') > 0) {
      int firstComma = message.indexOf(',');
      int secondComma = message.indexOf(',', firstComma + 1);

      int r = message.substring(0, firstComma).toInt();
      int g = message.substring(firstComma + 1, secondComma).toInt();
      int b = message.substring(secondComma + 1).toInt();

      setLedColor(r, g, b);
      Serial.printf("-> Cập nhật màu RGB: R=%d, G=%d, B=%d\n", r, g, b);
    } 
    // 2. Lệnh Bật (Trắng)
    else if (message == "ON" || message == "1") {
      setLedColor(255, 255, 255); // Sáng trắng
      Serial.println("-> Bật LED (Trắng)");
    } 
    // 3. Lệnh Tắt
    else if (message == "OFF" || message == "0") {
      setLedColor(0, 0, 0); // Tắt LED
      Serial.println("-> Tắt LED");
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Đang kết nối đến Wi-Fi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi đã kết nối thành công!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối đến MQTT...");
    String clientId = String(device_name) + String(student_id);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" Kết nối MQTT thành công!");
      client.subscribe(topic_led);
      Serial.print("Đã subscribe: ");
      Serial.println(topic_led);
    } else {
      Serial.print(" Thất bại, rc = ");
      Serial.print(client.state());
      Serial.println(". Thử lại sau 5 giây...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Khởi tạo LED RGB
  rgbLed.begin();
  rgbLed.setBrightness(50); // Đặt độ sáng (0 - 255) tránh bị chói mắt
  setLedColor(0, 0, 0);     // Ban đầu tắt LED

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  randomSeed(analogRead(0));
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop(); // Lắng nghe sự kiện MQTT

  // Gửi dữ liệu cảm biến định kỳ mỗi 5s
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    StaticJsonDocument<200> doc;
    doc["temp"] = random(250, 350) / 10.0;
    doc["hum"]  = random(400, 800) / 10.0;

    char payload[128];
    serializeJson(doc, payload);

    client.publish(topic_sensor, payload);
  }
}
