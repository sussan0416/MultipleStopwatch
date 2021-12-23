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

  prepareMainView();
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

void loopApp() {
  if (appState == Ota) {
    handleClientOta();
  }

  delay(10);
}

void onButtonClick(ButtonLabel label, ClickType type) {
  switch (appState) {
    case MainView:
      handleInMainView(label, type);
      break;
    case MenuView:
      handleInMenuView(label, type);
      break;
    case SelectTargetView:
      handleInSelectTargetView(label, type);
      break;
    case AdjustView:
      handleInAdjustView(label, type);
      break;
    case TaskTypeView:
      handleInTaskTypeView(label, type);
      break;
  }
}

// ----------------- Main -----------------

void prepareMainView() {
  appState = MainView;
  clearLcd();
  if (!countUpTicker.active()) {
    countUpTicker.attach(TICKER_COUNTUP_SEC, countUp);
  }
  if (!mainViewTicker.active()) {
    mainViewTicker.attach(TICKER_PRINT_SEC, printForTimer);
  }
  for (int i = 0; i < (sizeof(counterStates) / sizeof(counterStates[0])); i++) {
    setLed(i, counterStates[i] == Run);
  }
}

void handleInMainView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::R:
      prepareMenuView();
      break;

    case ButtonLabel::L:
      toggleDisplayBacklight();
      break;

    default:
      short index = convertButtonToIndex(label);
      if (type == ClickType::Long) {
        resetCounter(index);
        setLed(index, false);
      } else if (type == ClickType::Short) {
        if (counterStates[index] == Stop) {
          if (taskType == Single) {
            stopAllCounter();
            allLedOff();
          }
          startCounter(index);
          setLed(index, true);
        } else {
          stopCounter(index);
          setLed(index, false);
        }
      }
      break;
  }
}

// -------------- Menu -----------------

void prepareMenuView() {
  appState = MenuView;
  countUpTicker.detach();
  mainViewTicker.detach();
  stopAllCounter();
  allLedOff();
  delay(200);
  clearLcd();
  setCursorLcd(0, 0);
  printLcd("A: Adjust B: All 0");
  setCursorLcd(0, 1);

  char line[20];
  String taskTypeString;
  switch (taskType) {
    case Single:
      taskTypeString = "Single";
      break;
    case Multiple:
      taskTypeString = "Multi";
      break;
  }
  sprintf(line, "C: %-6s D: %-6s", taskTypeString, "");
  printLcd(line);

  setCursorLcd(0, 2);
  printLcd("E:");

  setCursorLcd(0, 3);
  printLcd("L: Update R: Return");
}

void handleInMenuView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      prepareSelectTargetView();
      break;

    case ButtonLabel::B:
      resetAllCounter();
      break;

    case ButtonLabel::C:
      prepareTaskTypeView();
      break;

    case ButtonLabel::L:
      prepareOtaMode();
      break;

    case ButtonLabel::R:
      prepareMainView();
      break;
  }
}

// ------------ Select Adjust -----------

void prepareSelectTargetView() {
  appState = SelectTargetView;
  clearLcd();
  setCursorLcd(0, 0);
  printLcd("Select a target:");
  setCursorLcd(10, 3);
  printLcd("R: Return");
}

void handleInSelectTargetView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      prepareAdjustView(0);
      break;
    case ButtonLabel::B:
      prepareAdjustView(1);
      break;
    case ButtonLabel::C:
      prepareAdjustView(2);
      break;
    case ButtonLabel::D:
      prepareAdjustView(3);
      break;
    case ButtonLabel::E:
      prepareAdjustView(4);
      break;
    case ButtonLabel::R:
      prepareMenuView();
      break;
  }
}

// ---------- Adjust -------------

void prepareAdjustView(int target) {
  appState = AdjustView;
  adjustTarget = target;
  clearLcd();
  setCursorLcd(0, 0);
  printLcd("Adjust:");
  printAdjustingValue();
  setCursorLcd(0, 1);
  printLcd("A: h+, B: m+, C: s+");
  setCursorLcd(0, 2);
  printLcd("D: h-, E: m-, L: s-");
  setCursorLcd(10, 3);
  printLcd("R: Return");
}

void printAdjustingValue() {
  unsigned long t = counterSeconds[adjustTarget] / 10;
  char lcd_str[10];
  sprintf(lcd_str, "%2d:%02d:%02d", t / 60 / 60, t / 60 % 60, t % 60);
  setCursorLcd(10, 0);
  printLcd(lcd_str);
}

void handleInAdjustView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      counterSeconds[adjustTarget] += 36000;
      printAdjustingValue();
      break;
    case ButtonLabel::B:
      counterSeconds[adjustTarget] += 600;
      printAdjustingValue();
      break;
    case ButtonLabel::C:
      counterSeconds[adjustTarget] += 10;
      printAdjustingValue();
      break;
    case ButtonLabel::D:
      if (counterSeconds[adjustTarget] >= 36000) {
        counterSeconds[adjustTarget] -= 36000;
        printAdjustingValue();
      }
      break;
    case ButtonLabel::E:
      if (counterSeconds[adjustTarget] >= 600) {
        counterSeconds[adjustTarget] -= 600;
        printAdjustingValue();
      }
      break;
    case ButtonLabel::L:
      if (counterSeconds[adjustTarget] >= 10) {
        counterSeconds[adjustTarget] -= 10;
        printAdjustingValue();
      }
      break;
    case ButtonLabel::R:
      adjustTarget = -1;
      prepareSelectTargetView();
      break;
  }
}

// ----------- TaskTypeView ------------

void prepareTaskTypeView() {
  appState = TaskTypeView;
  clearLcd();
  setCursorLcd(0, 0);
  printLcd("A: Single, B: Multi");
  setCursorLcd(10, 3);
  printLcd("R: Return");
}

void handleInTaskTypeView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      taskType = Single;
      break;
    case ButtonLabel::B:
      taskType = Multiple;
      break;
    case ButtonLabel::R:
      break;
  }
  prepareMenuView();
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

// ----------------- Ota --------------

void prepareOtaMode() {
  allLedOff();
  appState = Ota;
  clearLcd();
  setCursorLcd(0, 0);
  printLcd("OTA Mode");
  setCursorLcd(0, 1);
  printLcd("http://timer.local");
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
