#include <WiFi.h>
#include <time.h>

#include "Input.h"
#include "Output.h"
#include "AppState.h"
#include "CountupTimer.h"
#include "Ota.h"
#include "Env.h"

void setup() {
  Serial.begin(115200);
  setupInput();
  setupOutput();

  clearLcd();
  printLcd("Connecting to Wi-Fi");
  setupNetwork();

  clearLcd();
  printLcd("NTP Sync...");
  ntpSync();

  setupApp();
  clearLcd();
}

void loop() {
  loopInput();
  loopApp();
}

// ------------- input -------------

void setupInput() {
  for (int i = 0; i < (sizeof(buttons) / sizeof(buttons[0])); i++) {
    Button btn = buttons[i];
    pinMode(btn.pin, INPUT);
  }
}

void loopInput() {
  for (int i = 0; i < (sizeof(buttons) / sizeof(buttons[0])); i++) {
    Button &btn = buttons[i];

    if (digitalRead(btn.pin) == LOW) {
      if (btn.count < BUTTON_LONG_COUNT) {
        btn.count++;
      } else {
        if (!btn.isEventFired) {
          onButtonClick(btn.label, ClickType::Long);
          btn.isEventFired = true;
        }
      }
    } else {
      if (btn.count > BUTTON_SHORT_COUNT && btn.count < BUTTON_LONG_COUNT) {
        onButtonClick(btn.label, ClickType::Short);
      }

      btn.count = 0;
      btn.isEventFired = false;
    }
  }
}

// ------------ app ----------

void setupApp() {
  countUpTicker.attach(TICKER_COUNTUP_SEC, countUp);
  mainViewTicker.attach(TICKER_PRINT_SEC, printForTimer);
}

void loopApp() {
  if (appState == AppState::Ota) {
    handleClientOta();
  }

  delay(10);
}

void onButtonClick(ButtonLabel label, ClickType type) {
  switch (appState) {
    case AppState::MainView:
      handleInMainView(label, type);
  }
}

void handleInMainView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::R:
      countUpTicker.detach();
      mainViewTicker.detach();
      stopAllCounter();
      allLedOff();
      appState = AppState::Ota;
      clearLcd();
      setCursorLcd(0, 0);
      printLcd("OTA Mode");
      setCursorLcd(0, 1);
      printLcd("http://timer.local");
      break;

    case ButtonLabel::L:
      toggleDisplayBacklight();
      break;

    default:
      short index = convertButtonToIndex(label);
      LedLabel led = convertButtonToLed(label);
      if (type == ClickType::Long) {
        resetCounter(index);
        setLed(led, false);
      } else if (type == ClickType::Short) {
        if (counterStates[index] == Stop) {
          if (timerMode == Single) {
            stopAllCounter();
            allLedOff();
          }
          startCounter(index);
          setLed(led, true);
        } else {
          stopCounter(index);
          setLed(led, false);
        }
      }
      break;
  }
}

short convertButtonToIndex(ButtonLabel label) {
  switch (label) {
    case ButtonLabel::A:
      return 0;
    case ButtonLabel::B:
      return 1;
    case ButtonLabel::C:
      return 2;
    case ButtonLabel::D:
      return 3;
    case ButtonLabel::E:
      return 4;
    default:
      exit(1);
  }
}

LedLabel convertButtonToLed(ButtonLabel label) {
  switch (label) {
    case ButtonLabel::A:
      return LedLabel::A;
    case ButtonLabel::B:
      return LedLabel::B;
    case ButtonLabel::C:
      return LedLabel::C;
    case ButtonLabel::D:
      return LedLabel::D;
    case ButtonLabel::E:
      return LedLabel::E;
    default:
      exit(1);
  }
}

void printForTimer() {
  unsigned long sum = 0;
  for (int i = 0; i < (sizeof(counterSeconds) / sizeof(counterSeconds[0])); i++) {
    unsigned long t = counterSeconds[i] / 10;
    sum += t;

    unsigned short col = (i % 2 == 0) ? 0 : 10;
    unsigned short row = i / 2;
    setCursorLcd(col, row);

    char lcd_str[10];  // LCD has 20 chars per row. 20 / 2 = 10.
    sprintf(lcd_str, "%c%2d:%02d:%02d", buttonChars[i], t / 60 / 60, t / 60 % 60, t % 60);
    printLcd(lcd_str);
  }

  setCursorLcd(10, 2);
  printLcd("Total:");

  char str[10];
  if (sum < 60) {
    sprintf(str, "     :%02d", sum % 60);
  } else if (sum < 3600) {
    sprintf(str, "   %2d:%02d", sum / 60 % 60, sum % 60);
  } else {
    sprintf(str, "%2d:%02d:%02d", sum / 60 / 60, sum / 60 % 60, sum % 60);
  }
  setCursorLcd(11, 3);
  printLcd(str);

  setCursorLcd(1, 3);
  printLcd(getCurrentTime("%H:%M:%S"));
}

// ---------- Network ----------

#define TIMEZONE  "JST-9"
#define NTP_SERVER1   "ntp.nict.jp"

void setupNetwork() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }
}

void ntpSync() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  configTzTime(TIMEZONE, NTP_SERVER1);
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
}

String getCurrentTime(const char *format) {
  char destination[70];
  time_t now;
  struct tm *timeptr;

  now = time(nullptr);
  timeptr = localtime(&now);
  strftime(destination, sizeof(destination), format, timeptr);

  return String(destination);
}
