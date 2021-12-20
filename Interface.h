#include "CustomTypes.h"
#include <Arduino.h>

class Interface {
  public:
    void setup();
    void loop();
    void endLoop();
    int getNumberOfButtons();
    Button getButtonState(int index);
    void setButtonLighting(int index, bool isHigh);

    void clearLCD();
    void setCursorLCD(int col, int row);
    void printLCD(String str);
};
