#include "Interface.h"
#include <Arduino.h>

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

void Interface::setup() {
  for (int i = 0; i < (sizeof(button_pins) / sizeof(button_pins[0])); i++) {
    pinMode(button_pins[i], INPUT);
  }
  for (int i = 0; i < (sizeof(button_led_pins) / sizeof(button_led_pins[0])); i++) {
    pinMode(button_led_pins[i], OUTPUT);
  }
}

void Interface::loop() {
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
}

void Interface::setButtonLighting(int index, bool isHigh) {
  digitalWrite(button_led_pins[index], isHigh);
}

// ロジック移動していくための、一時的な退避
void Interface::endLoop() {
  // 処理の終わり
  for (int i = 0; i < (sizeof(button_states) / sizeof(button_states[0])); i++) {
    button_states[i] = Button::Up;
  }
}

// ロジック移動していくための、一時的な退避
int Interface::getNumberOfButtons() {
  // TODO: メイン以外のボタンも扱えるようにする
  return 5;
}

// ロジック移動していくための、一時的な退避
Button Interface::getButtonState(int index) {
  return button_states[index];
}
