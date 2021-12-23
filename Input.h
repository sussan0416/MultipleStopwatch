#define BUTTON_A_PIN 36
#define BUTTON_B_PIN 39
#define BUTTON_C_PIN 34
#define BUTTON_D_PIN 35
#define BUTTON_E_PIN 32
#define BUTTON_L_PIN 13
#define BUTTON_R_PIN 4

#define BUTTON_SHORT_COUNT 3
#define BUTTON_LONG_COUNT 200

enum class ButtonLabel {
  A, B, C, D, E, L, R
};

class Button {
  public:
    Button(int pin, ButtonLabel label):
      pin(pin),
      label(label){}

    int pin;
    unsigned int count = 0;
    ButtonLabel label;
    bool isEventFired = false;
};

enum class ClickType {
  Short, Long
};

Button btnA(BUTTON_A_PIN, ButtonLabel::A);
Button btnB(BUTTON_B_PIN, ButtonLabel::B);
Button btnC(BUTTON_C_PIN, ButtonLabel::C);
Button btnD(BUTTON_D_PIN, ButtonLabel::D);
Button btnE(BUTTON_E_PIN, ButtonLabel::E);
Button btnL(BUTTON_L_PIN, ButtonLabel::L);
Button btnR(BUTTON_R_PIN, ButtonLabel::R);

Button buttons[] = {btnA, btnB, btnC, btnD, btnE, btnL, btnR};
