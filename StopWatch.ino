#include <WiFi.h>
#include <time.h>

#include "Input.h"
#include "Output.h"
#include "AppState.h"
#include "CountupTimer.h"
#include "Schedule.h"
#include "Ota.h"
#include "Env.h"
#include "Slack.h"

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
      if (btn.isHolding) {
        if (btn.count < BUTTON_HOLD_COUNT) {
          btn.count++;
        } else {
          onButtonClick(btn.label, ClickType::Long);
          btn.count = 0;
        }
      } else {
        if (btn.count < BUTTON_LONG_COUNT) {
          btn.count++;
        } else {
          onButtonClick(btn.label, ClickType::Long);
          btn.count = 0;
          btn.isHolding = true;
        }
      }
    } else {
      if (!btn.isHolding && btn.count > BUTTON_SHORT_COUNT && btn.count < BUTTON_LONG_COUNT) {
        onButtonClick(btn.label, ClickType::Short);
      }

      btn.count = 0;
      btn.isHolding = false;
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
    case EditSelectView:
      handleInEditSelectView(label, type);
      break;
    case EditView:
      handleInEditView(label, type);
      break;
    case TaskTypeView:
      handleInTaskTypeView(label, type);
      break;
    case AlarmSelectView:
      handleInAlarmSelectView(label, type);
      break;
    case AlarmView:
      handleInAlarmView(label, type);
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
      int index = convertButtonToIndex(label);
      if (type == ClickType::Long) {
        resetCounter(index);
        setLed(index, false);
      } else if (type == ClickType::Short) {
        if (scheduleStates[index] == Present) {
          scheduleStates[index] = Past;
          resetRemainSeconds();
          isNotifyLedOn = false;
        }
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
  delay(100);

  setCursorLcd(0, 0);
  printLcd("A: Alarm  B: Edit");

  setCursorLcd(0, 1);
  printLcd("C: Report");

  if (taskType == Multiple) {
    setCursorLcd(0, 2);
    printLcd("E: Multi");
  }

  setCursorLcd(10, 3);
  printLcd("R: Return");
}

void handleInMenuView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      prepareAlarmSelectView();
      break;

    case ButtonLabel::B:
      prepareEditSelectView();
      break;

    case ButtonLabel::C:
      report();
      break;

    case ButtonLabel::E:
      if (type == ClickType::Long) {
        prepareTaskTypeView();
      }
      break;

    case ButtonLabel::L:
      if (type == ClickType::Long) {
        prepareOtaMode();
      }
      break;

    case ButtonLabel::R:
      prepareMainView();
      break;
  }
}

void report() {
  char a[10];
  char b[10];
  char c[10];
  char d[10];
  char e[10];
  char t[10];
  getCounterSeconds(0, a);
  getCounterSeconds(1, b);
  getCounterSeconds(2, c);
  getCounterSeconds(3, d);
  getCounterSeconds(4, e);
  getCounterSecondsSum(t);

  char destination[70];
  time_t now;
  struct tm *timeptr;

  now = time(nullptr);
  timeptr = localtime(&now);
  strftime(destination, sizeof(destination), "%Y-%m-%d %H:%M", timeptr);

  if (sendToSlack(destination, a, b, c, d, e, t)) {
    setCursorLcd(0, 1);
    printLcd("C: Done  ");
  } else {
    setCursorLcd(0, 1);
    printLcd("C: Error ");
  }
}

// ------------ Edit Select -----------

void prepareEditSelectView() {
  appState = EditSelectView;
  clearLcd();
  setCursorLcd(0, 0);
  printLcd("Edit");
  setCursorLcd(0, 1);
  printLcd("Select a target:");
  setCursorLcd(10, 3);
  printLcd("R: Return");
}

void handleInEditSelectView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      prepareEditView(0);
      break;
    case ButtonLabel::B:
      prepareEditView(1);
      break;
    case ButtonLabel::C:
      prepareEditView(2);
      break;
    case ButtonLabel::D:
      prepareEditView(3);
      break;
    case ButtonLabel::E:
      prepareEditView(4);
      break;
    case ButtonLabel::R:
      prepareMenuView();
      break;
  }
}

// ---------- Edit -------------

void prepareEditView(int target) {
  appState = EditView;
  adjustTarget = target;
  clearLcd();
  setCursorLcd(0, 0);
  printLcd(String(char(target + 65))); // 65: A, ..., 69: E
  printEditingValue();
  setCursorLcd(0, 1);
  printLcd("A: h+, B: m+, C: s+");
  setCursorLcd(0, 2);
  printLcd("D: h-, E: m-, L: s-");
  setCursorLcd(10, 3);
  printLcd("R: Return");
}

void printEditingValue() {
  unsigned long t = counterSeconds[adjustTarget] / 10;
  char lcd_str[10];
  sprintf(lcd_str, "%2d:%02d:%02d", t / 60 / 60, t / 60 % 60, t % 60);
  setCursorLcd(2, 0);
  printLcd(lcd_str);
}

void handleInEditView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      counterSeconds[adjustTarget] += 36000;
      printEditingValue();
      break;
    case ButtonLabel::B:
      counterSeconds[adjustTarget] += 600;
      printEditingValue();
      break;
    case ButtonLabel::C:
      counterSeconds[adjustTarget] += 10;
      printEditingValue();
      break;
    case ButtonLabel::D:
      if (counterSeconds[adjustTarget] >= 36000) {
        counterSeconds[adjustTarget] -= 36000;
        printEditingValue();
      }
      break;
    case ButtonLabel::E:
      if (counterSeconds[adjustTarget] >= 600) {
        counterSeconds[adjustTarget] -= 600;
        printEditingValue();
      }
      break;
    case ButtonLabel::L:
      if (counterSeconds[adjustTarget] >= 10) {
        counterSeconds[adjustTarget] -= 10;
        printEditingValue();
      }
      break;
    case ButtonLabel::R:
      adjustTarget = -1;
      prepareEditSelectView();
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
      prepareMenuView();
      break;
    case ButtonLabel::B:
      taskType = Multiple;
      prepareMenuView();
      break;
    case ButtonLabel::R:
      prepareMenuView();
      break;
  }
}

// ------------ Plan Select View ------------

void prepareAlarmSelectView() {
  appState = AlarmSelectView;
  clearLcd();
  setCursorLcd(0, 0);
  printLcd("Alarm");
  setCursorLcd(0, 1);
  printLcd("Select a target:");
  setCursorLcd(10, 3);
  printLcd("R: Return");
}

void handleInAlarmSelectView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      prepareAlarmView(0);
      break;
    case ButtonLabel::B:
      prepareAlarmView(1);
      break;
    case ButtonLabel::C:
      prepareAlarmView(2);
      break;
    case ButtonLabel::D:
      prepareAlarmView(3);
      break;
    case ButtonLabel::E:
      prepareAlarmView(4);
      break;
    case ButtonLabel::R:
      prepareMenuView();
      break;
  }
}

// ------------- Alarm View

void prepareAlarmView(int target) {
  appState = AlarmView;

  adjustTarget = target;
  time_t now;
  struct tm *timeptr;
  now = time(nullptr);
  timeptr = localtime(&now);
  scheduleTimeSeconds[adjustTarget] = 60 * 60 * timeptr->tm_hour + 60 * timeptr->tm_min;

  clearLcd();
  setCursorLcd(0, 0);
  printLcd(String(char(target + 65))); // 65: A, ..., 69: E
  printAlarmValue();
  setCursorLcd(0, 1);
  printLcd("A: h+, B: m+, C: s+");
  setCursorLcd(0, 2);
  printLcd("D: h-, E: m-, L: s-");
  setCursorLcd(10, 3);
  printLcd("R: Return");
}

void printAlarmValue() {
  unsigned long t = scheduleTimeSeconds[adjustTarget];
  char lcd_str[10];
  sprintf(lcd_str, "%2d:%02d:%02d", t / 60 / 60, t / 60 % 60, t % 60);
  setCursorLcd(2, 0);
  printLcd(lcd_str);
}

void handleInAlarmView(ButtonLabel label, ClickType type) {
  switch (label) {
    case ButtonLabel::A:
      if (scheduleTimeSeconds[adjustTarget] <= 24 * 60 * 60 - 3600 - 1) {
        scheduleTimeSeconds[adjustTarget] += 3600;
        printAlarmValue();
      }
      break;
    case ButtonLabel::B:
      if (scheduleTimeSeconds[adjustTarget] <= 24 * 60 * 60 - 60 - 1) {
        scheduleTimeSeconds[adjustTarget] += 60;
        printAlarmValue();
      }
      break;
    case ButtonLabel::C:
      if (scheduleTimeSeconds[adjustTarget] <= 24 * 60 * 60 - 1 - 1) {
        scheduleTimeSeconds[adjustTarget] += 1;
        printAlarmValue();
      }
      break;
    case ButtonLabel::D:
      if (scheduleTimeSeconds[adjustTarget] >= 3600) {
        scheduleTimeSeconds[adjustTarget] -= 3600;
        printAlarmValue();
      }
      break;
    case ButtonLabel::E:
      if (scheduleTimeSeconds[adjustTarget] >= 60) {
        scheduleTimeSeconds[adjustTarget] -= 60;
        printAlarmValue();
      }
      break;
    case ButtonLabel::L:
      if (scheduleTimeSeconds[adjustTarget] >= 1) {
        scheduleTimeSeconds[adjustTarget] -= 1;
        printAlarmValue();
      }
      break;
    case ButtonLabel::R:
      time_t now;
      struct tm *timeptr;

      now = time(nullptr);
      timeptr = localtime(&now);

      unsigned long t = scheduleTimeSeconds[adjustTarget];
      int hour = t / 60 / 60;
      int minute = t / 60 % 60;
      int second = t % 60;
      timeptr->tm_hour = hour;
      timeptr->tm_min = minute;
      timeptr->tm_sec = second;

      time_t scheduled = mktime(timeptr);
      schedules[adjustTarget] = scheduled;
      scheduleStates[adjustTarget] = (scheduled - now > 0) ? Future : Past;
      resetRemainSeconds();

      adjustTarget = -1;
      prepareAlarmSelectView();
      break;
  }
}

int convertButtonToIndex(ButtonLabel label) {
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
  printLcd("OTA Update Mode");
  setCursorLcd(0, 1);
  printLcd("http://timer.local");
}

void printForTimer() {
  // 現在時刻
  setCursorLcd(0, 0);
  printLcd(getCurrentTime("%H:%M:%S"));

  // 各計測値
  unsigned long sum = 0;
  for (int i = 0; i < (sizeof(counterSeconds) / sizeof(counterSeconds[0])); i++) {
    unsigned long t = counterSeconds[i] / 10;
    sum += t;

    unsigned int col = (i % 2 == 0) ? 0 : 11;
    unsigned int row = i / 2 + 1;
    setCursorLcd(col, row);

    char lcd_str[9];  // LCD has 20 chars per row. 20 / 2 = 10.
    sprintf(lcd_str, "%c%2d:%02d:%02d", buttonChars[i], t / 60 / 60, t / 60 % 60, t % 60);
    printLcd(lcd_str);
  }

  // 合計時間
  char str[10];
  sprintf(str, "%d:%02d", sum / 60 / 60, sum / 60 % 60);
  char totalTime[9];
  sprintf(totalTime, "%9s", str);
  setCursorLcd(11, 3);
  printLcd(totalTime);

  // アラーム残り時間
  for (int i = 0; i < (sizeof(scheduleStates) / sizeof(scheduleStates[0])); i++) {
    if (scheduleStates[i] == Past) { continue; }

    int remainSeconds = schedules[i] - time(nullptr);
    if (remainSecondsForDisplay >= remainSeconds) {
      remainSecondsForDisplay = remainSeconds;
      nextAlarmIndex = i;
    }

    if (scheduleStates[i] == Present) { continue; }
    if (schedules[i] - time(nullptr) > 0) { continue; }  // Future State

    scheduleStates[i] = Present;
  }

  if (nextAlarmIndex != -1) {
    setCursorLcd(9, 0);
    writeLcd(arrowChar);
    setCursorLcd(10, 0);
    printLcd(String(buttonChars[nextAlarmIndex]));

    char remaiTimeStr[8];
    if (remainSecondsForDisplay != remainSecondsDefaultValue) {
      int remain = abs(remainSecondsForDisplay);
      if (remain < 60) {
        sprintf(remaiTimeStr, ":%02d", remain % 60);
      } else if (remain < 3600) {
        sprintf(remaiTimeStr, "%d:%02d", remain / 60 % 60, remain % 60);
      } else {
        sprintf(remaiTimeStr, "%d:%02d:%02d", remain / 60 / 60, remain / 60 % 60, remain % 60);
      }
    }
    char remainTime[8];
    sprintf(remainTime, "%-8s", remaiTimeStr);
    setCursorLcd(12, 0);
    printLcd(remainTime);
  } else {
    setCursorLcd(9, 0);
    printLcd("           ");
  }

  // アラームのLED点灯
  for (int i = 0; i < (sizeof(scheduleStates) / sizeof(scheduleStates[0])); i++) {
    if (scheduleStates[i] != Present) { continue; }
    setLed(i, isNotifyLedOn);
  }
  isNotifyLedOn = !isNotifyLedOn;
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
