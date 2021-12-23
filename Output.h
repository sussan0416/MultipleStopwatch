#include <LiquidCrystal.h>

enum class LedLabel {
  A, B, C, D, E
};

class Led {
  public:
    Led(int pin, LedLabel label):
      pin(pin),
      label(label){}

    int pin;
    LedLabel label;
};

// --------- button led --------

#define BUTTON_A_LED_OUT_PIN 33
#define BUTTON_B_LED_OUT_PIN 25
#define BUTTON_C_LED_OUT_PIN 26
#define BUTTON_D_LED_OUT_PIN 27
#define BUTTON_E_LED_OUT_PIN 14

Led ledA(BUTTON_A_LED_OUT_PIN, LedLabel::A);
Led ledB(BUTTON_B_LED_OUT_PIN, LedLabel::B);
Led ledC(BUTTON_C_LED_OUT_PIN, LedLabel::C);
Led ledD(BUTTON_D_LED_OUT_PIN, LedLabel::D);
Led ledE(BUTTON_E_LED_OUT_PIN, LedLabel::E);

Led leds[] = {ledA, ledB, ledC, ledD, ledE};

// ---------- lcd -------------

#define LCD_RS_PIN 23
#define LCD_EN_PIN 22
#define LCD_D4_PIN 21
#define LCD_D5_PIN 19
#define LCD_D6_PIN 18
#define LCD_D7_PIN 17

#define LCD_ROW 4
#define LCD_COL 20

#define BACKLIGHT_PIN 16

LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);
bool isBacklightOn = false;

void setupOutput() {
  for (int i = 0; i < (sizeof(leds) / sizeof(leds[0])); i++) {
    Led led = leds[i];
    pinMode(led.pin, OUTPUT);
  }

  pinMode(BACKLIGHT_PIN, OUTPUT);

  lcd.begin(LCD_COL, LCD_ROW);
}

void setLed(LedLabel label, bool isHigh) {
  switch (label) {
    case LedLabel::A:
      digitalWrite(ledA.pin, isHigh);
      break;
    case LedLabel::B:
      digitalWrite(ledB.pin, isHigh);
      break;
    case LedLabel::C:
      digitalWrite(ledC.pin, isHigh);
      break;
    case LedLabel::D:
      digitalWrite(ledD.pin, isHigh);
      break;
    case LedLabel::E:
      digitalWrite(ledE.pin, isHigh);
      break;
  }
}

void allLedOff() {
  for (int i = 0; i < (sizeof(leds) / sizeof(leds[0])); i++) {
    digitalWrite(leds[i].pin, LOW);
  }
}

void toggleDisplayBacklight() {
  isBacklightOn = !isBacklightOn;
  digitalWrite(BACKLIGHT_PIN, isBacklightOn);
}

void clearLcd() {
  lcd.clear();
}

void setCursorLcd(int col, int row) {
  lcd.setCursor(col, row);
}

void printLcd(String str) {
  lcd.print(str);
}
