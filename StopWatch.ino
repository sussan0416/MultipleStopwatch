#include "CustomTypes.h"
#include <Ticker.h>
#include <LiquidCrystal.h>
#include "Backlight.h"

#define BUTTON_1_PIN 36
#define BUTTON_2_PIN 39
#define BUTTON_3_PIN 34
#define BUTTON_4_PIN 35
#define BUTTON_5_PIN 32

#define BUTTON_1_LED_OUT_PIN 33
#define BUTTON_2_LED_OUT_PIN 25
#define BUTTON_3_LED_OUT_PIN 26
#define BUTTON_4_LED_OUT_PIN 27
#define BUTTON_5_LED_OUT_PIN 14

#define LCD_RS_PIN 23
#define LCD_EN_PIN 22
#define LCD_D4_PIN 21
#define LCD_D5_PIN 19
#define LCD_D6_PIN 18
#define LCD_D7_PIN 17

#define LCD_ROW 4
#define LCD_COL 20

#define TICKER_MEASURE_PERIOD_SEC 0.1
#define TICKER_PRINT_PERIOD_SEC 0.2

// Measurement
unsigned long measured_seconds[] = {0, 0, 0, 0, 0};
Measure measure_states[] = {
  Measure::Stop, Measure::Stop,
  Measure::Stop, Measure::Stop,
  Measure::Stop
};

// Buttons
const char button_chars[] = {'A', 'B', 'C', 'D', 'E'};
short button_counts[] = {0, 0, 0, 0, 0};
Button button_states[] = {
  Button::Up, Button::Up,
  Button::Up, Button::Up,
  Button::Up
};
int button_pins[] = {
  BUTTON_1_PIN, BUTTON_2_PIN,
  BUTTON_3_PIN, BUTTON_4_PIN, BUTTON_5_PIN
};
int button_led_pins[] = {
  BUTTON_1_LED_OUT_PIN,
  BUTTON_2_LED_OUT_PIN,
  BUTTON_3_LED_OUT_PIN,
  BUTTON_4_LED_OUT_PIN,
  BUTTON_5_LED_OUT_PIN
};
const short button_short_count = 3;
const short button_long_count = 200;

// Backlight
Backlight backlight;

// Tickers
Ticker measurement_ticker;
Ticker print_measure_ticker;

// LCD
LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

// fired every 1/10 second.
void measure() {
  for (int i = 0; i < (sizeof(measure_states) / sizeof(measure_states[0])); i++) {
    if (measure_states[i] == Measure::Run) {
      measured_seconds[i]++;
    }
  }
}

void printMeasure() {
  for (int i = 0; i < (sizeof(measure_states) / sizeof(measure_states[0])); i++) {
    unsigned long t = measured_seconds[i] / 10;
    char str[32];
    sprintf(str, "%d: %02d:%02d:%02d, ", i, t / 60 / 60, t / 60 % 60, t % 60);
    Serial.print(str);

    unsigned short col = (i % 2 == 0) ? 0 : 10;
    unsigned short row = i / 2;
    lcd.setCursor(col, row);

    char lcd_str[10];  // LCD has 20 chars per row. 20 / 2 = 10.
    sprintf(lcd_str, "%c%02d:%02d:%02d", button_chars[i], t / 60 / 60, t / 60 % 60, t % 60);
    lcd.print(lcd_str);
  }
  Serial.println("");
}

void stopAllMeasure() {
  for (int i = 0; i < (sizeof(measure_states) / sizeof(measure_states[0])); i++) {
    measure_states[i] = Measure::Stop;
  }
}

void setup() {
  Serial.begin(115200);
  
  for (int i = 0; i < (sizeof(button_pins) / sizeof(button_pins[0])); i++) {
    pinMode(button_pins[i], INPUT);
  }
  for (int i = 0; i < (sizeof(button_led_pins) / sizeof(button_led_pins[0])); i++) {
    pinMode(button_led_pins[i], OUTPUT);
  }

  backlight.begin();

  lcd.begin(LCD_COL, LCD_ROW);

  // measure every 0.1 seconds.
  measurement_ticker.attach(TICKER_MEASURE_PERIOD_SEC, measure);
  print_measure_ticker.attach(TICKER_PRINT_PERIOD_SEC, printMeasure);
}

void loop() {
  // ボタン状態のチェック
  for (int i = 0; i < (sizeof(button_pins) / sizeof(button_pins[0])); i++) {
    if (digitalRead(button_pins[i]) == LOW) {
      if (button_counts[i] < button_long_count) {
        button_counts[i]++;
      } else {
        button_states[i] = Button::DownLong;
      }
    } else {
      if (button_counts[i] > button_short_count && button_counts[i] < button_long_count) {
        button_states[i] = Button::DownShort;
      }
      button_counts[i] = 0;
    }
  }

  // 計測ステートの変更
  for (int i = 0; i < (sizeof(button_pins) / sizeof(button_pins[0])); i++) {
    switch (button_states[i]) {
      case Button::Up:
        // なし
        break;
      case Button::DownShort:
        // ボタンクリック時の処理
        switch (measure_states[i]) {
          case Measure::Stop:
            stopAllMeasure();
            measure_states[i] = Measure::Run;
            break;
          case Measure::Run:
            measure_states[i] = Measure::Stop;
            break;
        }
        break;
      case Button::DownLong:
        // ボタン長押し時の処理
        measure_states[i] = Measure::Stop;
        measured_seconds[i] = 0;
        break;
    }
  }

  // 処理
  for (int i = 0; i < (sizeof(measure_states) / sizeof(measure_states[0])); i++) {
    switch (measure_states[i]) {
      case Measure::Stop:
        digitalWrite(button_led_pins[i], LOW);
        break;
      case Measure::Run:
        digitalWrite(button_led_pins[i], HIGH);
        break;
    }
  }

  // 処理の終わり
  for (int i = 0; i < (sizeof(button_states) / sizeof(button_states[0])); i++) {
    button_states[i] = Button::Up;
  }

  // バックライト
  backlight.onLoop();

  delay(10);
}
