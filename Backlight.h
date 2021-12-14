class Backlight {
  public:
    void begin();
    void onLoop();
  private:
    bool is_lighting = false;
    short backlight_button_count = 0;
};
