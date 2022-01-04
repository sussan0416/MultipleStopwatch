#include <LiquidCrystal.h>

enum LedLabel {
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

Led ledA(BUTTON_A_LED_OUT_PIN, A);
Led ledB(BUTTON_B_LED_OUT_PIN, B);
Led ledC(BUTTON_C_LED_OUT_PIN, C);
Led ledD(BUTTON_D_LED_OUT_PIN, D);
Led ledE(BUTTON_E_LED_OUT_PIN, E);

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

int arrowChar = 0;
byte arrow[8] = {
  B00000,
  B01000,
  B01100,
  B01110,
  B01100,
  B01000,
  B00000,
};

void setupOutput() {
  for (int i = 0; i < (sizeof(leds) / sizeof(leds[0])); i++) {
    Led led = leds[i];
    pinMode(led.pin, OUTPUT);
  }

  pinMode(BACKLIGHT_PIN, OUTPUT);

  lcd.begin(LCD_COL, LCD_ROW);
  delay(10);
  lcd.createChar(arrowChar, arrow);
  delay(10);
}

void setLed(int index, bool isHigh) {
  switch (index) {
    case 0:
      digitalWrite(ledA.pin, isHigh);
      break;
    case 1:
      digitalWrite(ledB.pin, isHigh);
      break;
    case 2:
      digitalWrite(ledC.pin, isHigh);
      break;
    case 3:
      digitalWrite(ledD.pin, isHigh);
      break;
    case 4:
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

void writeLcd(int index) {
  lcd.write(byte(index));
}
