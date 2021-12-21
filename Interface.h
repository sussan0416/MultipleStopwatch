#include "CustomTypes.h"
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

#define LCD_RS_PIN 23
#define LCD_EN_PIN 22
#define LCD_D4_PIN 21
#define LCD_D5_PIN 19
#define LCD_D6_PIN 18
#define LCD_D7_PIN 17

#define LCD_ROW 4
#define LCD_COL 20

#define BUTTON_BACKLIGHT_PIN 13
#define BACKLIGHT_PIN 16

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
