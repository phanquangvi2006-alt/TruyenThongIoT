#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <coap-simple.h>

/* ========== CONFIG ========== */
const char* WIFI_SSID = "CAU VI";     // <--- SỬA: Tên WiFi nhà bạn
const char* WIFI_PASS = "0968978073";       // <--- SỬA: Mật khẩu WiFi nhà bạn

/* Địa chỉ IP của Máy tính đang chạy Node.js Server */
IPAddress coap_server_ip(192, 168, 1, 19);       // <--- SỬA: Thay X bằng IP thật của máy tính
const int coap_port = 5683;

/* CoAP Resource */
const char* coap_resource = "sensor/temp";
/* Tần suất gửi (ms) */
const unsigned long SEND_INTERVAL = 5000;
/* ============================ */

WiFiUDP udp;
Coap coap(udp);

unsigned long lastSend = 0;

// Hàm callback nhận phản hồi từ Server (để xem Server có rep hay không)
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = '\0';
  
  Serial.print("\n[ACK] Nhận phản hồi từ Server: ");
  Serial.println(p);
}

void setup_wifi() {
  Serial.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP của Kit ESP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  // Khai báo hàm nhận phản hồi từ server
  coap.response(callback_response);
  coap.start();
  Serial.println("CoAP client started");
}

void loop() {
  coap.loop(); // Giữ kết nối mạng và xử lý các gói tin phản hồi

  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    String payload = "temp=30.5";

    Serial.println("------------------------------------");
    Serial.print("Sending CoAP CON POST to ");
    Serial.println(coap_resource);

    // ĐÃ SỬA: Chuyển COAP_NON -> COAP_CON để kích hoạt yêu cầu ACK
    // ĐÃ SỬA: Thêm NULL, 0 vào vị trí tham số thứ 6 và 7 để khớp với thư viện của bạn
    coap.send(
      coap_server_ip,
      coap_port,
      coap_resource,
      COAP_CON,                   // Tham số 4: Kiểu CON (yêu cầu ACK)
      COAP_POST,                  // Tham số 5: Phương thức POST
      NULL,                       // Tham số 6: Token (Để trống là NULL)
      0,                          // Tham số 7: Token length (Độ dài bằng 0)
      (uint8_t*)payload.c_str(),  // Tham số 8: Payload dữ liệu
      payload.length()            // Tham số 9: Độ dài dữ liệu
    );
  }
}

