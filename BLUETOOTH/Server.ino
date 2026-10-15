#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_NeoPixel.h>

// UUIDs cho BLE
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Chân LED tích hợp trên ESP32-S3 (thường là GPIO 48, thử đổi nếu không sáng)
#define LED_PIN 48
#define NUMPIXELS 1   // chỉ có 1 LED RGB tích hợp

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.length() > 0) {
      Serial.print("Received: ");
      Serial.println(value);

      if (value == "run") {
        pixels.setPixelColor(0, pixels.Color(0, 1, 0)); // xanh lá
        pixels.show();
        Serial.println("LED ON (green)");
      } else if (value == "stop") {
        pixels.clear();
        pixels.show();
        Serial.println("LED OFF");
      } else if (value == "red") {
        pixels.setPixelColor(0, pixels.Color(50, 0, 0));
        pixels.show();
        Serial.println("LED RED");
      } else if (value == "blue") {
        pixels.setPixelColor(0, pixels.Color(0, 0, 50));
        pixels.show();
        Serial.println("LED BLUE");
      } else if (value == "green") {
        pixels.setPixelColor(0, pixels.Color(0, 50, 0));
        pixels.show();
        Serial.println("LED GREEN");
      }
      else if (value == "white") {
        pixels.setPixelColor(0, pixels.Color(50, 50, 50));
        pixels.show();
        Serial.println("LED white");
      }
      else if (value == "yellow") {
        pixels.setPixelColor(0, pixels.Color(50, 50, 0));
        pixels.show();
        Serial.println("LED yellow");
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.clear();
  pixels.show();

  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32S3 BLE LED");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic =
    pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_WRITE
    );

  pCharacteristic->setValue("Send 'run', 'stop', 'red', 'green', 'blue','all'");
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE server ready! Use your phone to send commands.");
}

void loop() {
  delay(2000);
}
