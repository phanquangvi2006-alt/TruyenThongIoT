#define ESP_RX 18
#define ESP_TX 19

HardwareSerial ArduinoSerial(2);

void setup() {
  Serial.begin(115200);

  ArduinoSerial.begin(9600, SERIAL_8N1, ESP_RX, ESP_TX);

  Serial.println("Nhap 0 de tat LED");
  Serial.println("Nhap 1 de bat LED");
}

void loop() {
  if (Serial.available()) {
    char data = Serial.read();

    if (data == '0' || data == '1') {
      ArduinoSerial.write(data);

      Serial.print("Da gui: ");
      Serial.println(data);
    }
  }

  if (ArduinoSerial.available()) {
    char data = ArduinoSerial.read();

    Serial.print("Arduino phan hoi: ");
    Serial.println(data);
  }
}
