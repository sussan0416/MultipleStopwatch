#include "Backlight.h"
#include "Constants.h"
#include <Arduino.h>

#define BUTTON_BACKLIGHT_PIN 13
#define BACKLIGHT_PIN 16

void Backlight::begin() {
  pinMode(BUTTON_BACKLIGHT_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
}

void Backlight::onLoop() {
  if (digitalRead(BUTTON_BACKLIGHT_PIN) == LOW) {
    backlight_button_count++;
  } else {
    if (backlight_button_count > BUTTON_SHORT_COUNT) {
      is_lighting = !is_lighting;
    }
    backlight_button_count = 0;
  }
  digitalWrite(BACKLIGHT_PIN, is_lighting);
}
