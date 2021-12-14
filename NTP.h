#include <Arduino.h>

class NTP {
  public:
    void begin();
    String getCurrentTime(const char *format = "%Y/%m/%d %H:%M:%S");
};
