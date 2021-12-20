#include "Interface.h"
#include <Ticker.h>
#include "Backlight.h"
#include "NTP.h"
#include "Env.h"
#include <WiFi.h>
#include "OTA.h"

#define MODE_BUTTON_PIN 4

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

// Backlight
Backlight backlight;

// Tickers
Ticker measurement_ticker;
Ticker print_measure_ticker;

// NTP
NTP ntp;

// OTA
OTA ota;
bool is_ota_mode;

Interface interface;

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
    sprintf(str, "%d: %2d:%02d:%02d, ", i, t / 60 / 60, t / 60 % 60, t % 60);
    Serial.print(str);

    unsigned short col = (i % 2 == 0) ? 0 : 10;
    unsigned short row = i / 2;
    interface.setCursorLCD(col, row);

    char lcd_str[10];  // LCD has 20 chars per row. 20 / 2 = 10.
    sprintf(lcd_str, "%c%2d:%02d:%02d", button_chars[i], t / 60 / 60, t / 60 % 60, t % 60);
    interface.printLCD(lcd_str);
  }
  Serial.println("");
  interface.setCursorLCD(0, 3);
  interface.printLCD(ntp.getCurrentTime());
}

void stopAllMeasure() {
  for (int i = 0; i < (sizeof(measure_states) / sizeof(measure_states[0])); i++) {
    measure_states[i] = Measure::Stop;
  }
}

void setup() {
  interface.setup();
  
  Serial.begin(115200);

  backlight.begin();

  interface.printLCD("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }

  interface.clearLCD();
  interface.printLCD("NTP Sync...");
  ntp.begin();
  interface.clearLCD();

  // measure every 0.1 seconds.
  measurement_ticker.attach(TICKER_MEASURE_PERIOD_SEC, measure);
  print_measure_ticker.attach(TICKER_PRINT_PERIOD_SEC, printMeasure);
}

void loop() {
  // OTA
  if (digitalRead(MODE_BUTTON_PIN) == LOW) {
    is_ota_mode = true;
    measurement_ticker.detach();
    print_measure_ticker.detach();

    interface.clearLCD();
    interface.setCursorLCD(0, 0);
    interface.printLCD("OTA Mode");
    interface.setCursorLCD(0, 1);
    interface.printLCD("http://timer.local");
  }

  if (is_ota_mode) {
    ota.handleClient();
    return;
  }

  interface.loop();

  // 計測ステートの変更
  for (int i = 0; i < interface.getNumberOfButtons(); i++) {
    switch (interface.getButtonState(i)) {
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
        interface.setButtonLighting(i, false);
        break;
      case Measure::Run:
        interface.setButtonLighting(i, true);
        break;
    }
  }

  interface.endLoop();

  // バックライト
  backlight.onLoop();

  delay(10);
}
