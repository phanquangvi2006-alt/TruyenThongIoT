#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define BTN_MODE 2
#define BTN_SET  3
#define BTN_UP   5
#define BTN_DOWN 6
#define PIN_LED  8
#define PIN_BUZZ 9

LiquidCrystal_I2C lcd(0x20, 16, 2);
RTC_DS1307 rtc;

byte mode = 0, step = 0;
bool ringing = false;
byte alarmH = 6, alarmM = 30;
bool alarmOn = true;
byte tH, tM, tS;
DateTime now;
unsigned long tRead = 0, tDraw = 0, tRing = 0;
int lastKey = -1;

// MODE nhấn giữ/nhấn nhanh
unsigned long modePressStart = 0;
bool modePressed = false;

const char *NAME_ALARM[2] = {"Gio ", "Phut"};
const char *NAME_TIME[3]  = {"Gio ", "Phut", "Giay"};

void saveAlarm() {
  EEPROM.update(0, 0x5A);
  EEPROM.update(1, alarmH);
  EEPROM.update(2, alarmM);
  EEPROM.update(3, alarmOn);
}

void loadAlarm() {
  if (EEPROM.read(0) == 0x5A) {
    alarmH  = EEPROM.read(1);
    alarmM  = EEPROM.read(2);
    alarmOn = EEPROM.read(3);
  }
  if (alarmH > 23) alarmH = 6;
  if (alarmM > 59) alarmM = 30;
}

void showLine(byte row, const char *s) {
  lcd.setCursor(0, row);
  byte n = 0;
  while (s[n] && n < 16) { lcd.print(s[n]); n++; }
  while (n < 16) { lcd.print(' '); n++; }
}

bool readButton(byte pin) {
  if (digitalRead(pin) == LOW) {
    delay(50);
    if (digitalRead(pin) == LOW) {
      while (digitalRead(pin) == LOW);
      return true;
    }
  }
  return false;
}

void setup() {
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_SET,  INPUT_PULLUP);
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  pinMode(PIN_LED,  OUTPUT); digitalWrite(PIN_LED, HIGH);
  pinMode(PIN_BUZZ, OUTPUT); digitalWrite(PIN_BUZZ, HIGH);

  lcd.init();
  lcd.backlight();
  rtc.begin();

  now = rtc.now();
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    now = rtc.now();
  }
  loadAlarm();
}

void loop() {
  char buf[17];

  if (millis() - tRead >= 500) {
    tRead = millis();
    now = rtc.now();
  }

  int key = now.hour() * 60 + now.minute();
  if (key != lastKey) {
    lastKey = key;
    if (alarmOn && !ringing && now.hour() == alarmH && now.minute() == alarmM) {
      ringing = true;
      tRing = millis();
      tone(PIN_BUZZ, 2500);
      digitalWrite(PIN_LED, LOW);
    }
  }

  if (ringing) {
    if (millis() - tRing >= 30000) {
      ringing = false;
      noTone(PIN_BUZZ);
      digitalWrite(PIN_BUZZ, HIGH);
      digitalWrite(PIN_LED, HIGH);
    }
  }
  else {
    // xử lý MODE nhấn nhanh/nhấn giữ
    if (digitalRead(BTN_MODE) == LOW) {
      if (!modePressed) {
        modePressed = true;
        modePressStart = millis();
      }
    } else {
      if (modePressed) {
        unsigned long duration = millis() - modePressStart;
        if (duration > 1000) {
          // nhấn giữ >2s → đổi AL ON/OFF
          alarmOn = !alarmOn;
          saveAlarm();
        } else {
          // nhấn nhanh → chuyển chế độ
          mode = (mode + 1) % 3;
          step = 0;
          if (mode == 1) { tH = alarmH; tM = alarmM; }
          if (mode == 2) { tH = now.hour(); tM = now.minute(); tS = now.second(); }
        }
        modePressed = false;
      }
    }

    // xử lý UP/DOWN/SET
    int d = 0;
    if (readButton(BTN_UP))   d = 1;
    if (readButton(BTN_DOWN)) d = -1;
    bool set = false;
    if (readButton(BTN_SET)) set = true;

    if (mode == 1) {
      if (d) {
        if (step == 0) tH = (tH + 24 + d) % 24;
        else           tM = (tM + 60 + d) % 60;
      }
      if (set) {
        if (step == 0) step = 1;
        else {
          alarmH = tH; alarmM = tM;
          saveAlarm();
          lastKey = -1;
          mode = 0;
        }
      }
    }
    else if (mode == 2) {
      if (d) {
        if (step == 0)      tH = (tH + 24 + d) % 24;
        else if (step == 1) tM = (tM + 60 + d) % 60;
        else                tS = (tS + 60 + d) % 60;
      }
      if (set) {
        if (step < 2) step++;
        else {
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), tH, tM, tS));
          lastKey = -1;
          mode = 0;
        }
      }
    }
  }

  if (millis() - tDraw >= 200) {
    tDraw = millis();
    if (ringing) {
      showLine(0, "*** BAO THUC ***");
      showLine(1, "Dang keu...");
    }
    else if (mode == 0) {
      sprintf(buf, "%02d:%02d:%02d  %s",
              now.hour(), now.minute(), now.second(),
              alarmOn ? "AL ON" : "AL OFF");
      showLine(0, buf);
      sprintf(buf, "%02d/%02d/%04d %02d:%02d",
              now.day(), now.month(), now.year(), alarmH, alarmM);
      showLine(1, buf);
    }
    else if (mode == 1) {
      showLine(0, "CAI BAO THUC");
      sprintf(buf, "%s   %02d:%02d", NAME_ALARM[step], tH, tM);
      showLine(1, buf);
    }
    else {
      showLine(0, "CAI THOI GIAN");
      sprintf(buf, "%s %02d:%02d:%02d", NAME_TIME[step], tH, tM, tS);
      showLine(1, buf);
    }
  }
}
