#include "CustomTypes.h"
#include "Ticker.h"

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

// Measurement
unsigned long measured_seconds[] = {0, 0, 0, 0, 0};
Measure measure_states[] = {
  Measure::Stop, Measure::Stop,
  Measure::Stop, Measure::Stop,
  Measure::Stop
};

// Buttons
// 0 - 4: Working
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

// Tickers
Ticker measurement_ticker;
Ticker print_measure_ticker;

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
    pinMode(button_pins[i], INPUT_PULLUP);
  }
  for (int i = 0; i < (sizeof(button_led_pins) / sizeof(button_led_pins[0])); i++) {
    pinMode(button_led_pins[i], OUTPUT);
  }

  // measure every 0.1 seconds.
  measurement_ticker.attach(0.1, measure);
  print_measure_ticker.attach(1.0, printMeasure);
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

  delay(10);
}
