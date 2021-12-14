#include "NTP.h"
#include <WiFi.h>
#include <time.h>
#include "Env.h"

#define TIMEZONE  "JST-9"
#define NTP_SERVER1   "ntp.nict.jp"

void NTP::begin() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }
  
  configTzTime(TIMEZONE, NTP_SERVER1);
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

String NTP::getCurrentTime(const char *format) {
  char destination[70];
  time_t now;
  struct tm *timeptr;

  now = time(nullptr);
  timeptr = localtime(&now);
  strftime(destination, sizeof(destination), format, timeptr);

  return String(destination);
}
