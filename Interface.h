#include "CustomTypes.h"

class Interface {
  public:
    void setup();
    void loop();
    void endLoop();
    int getNumberOfButtons();
    Button getButtonState(int index);
    void setButtonLighting(int index, bool isHigh);
};
