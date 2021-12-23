#include <Ticker.h>

enum CounterState {
  Stop, Run
};

enum TimerMode {
  Single, Multiple
};

TimerMode timerMode = Single;

#define TICKER_COUNTUP_SEC 0.1
#define TICKER_PRINT_SEC 0.2

// Tickers
Ticker countUpTicker;
Ticker mainViewTicker;

unsigned long counterSeconds[] = {0, 0, 0, 0, 0};
CounterState counterStates[] = {Stop, Stop, Stop, Stop, Stop};
const char buttonChars[] = {'A', 'B', 'C', 'D', 'E'};

void resetCounter(short index) {
  counterSeconds[index] = 0;
  counterStates[index] = Stop;
}

void startCounter(short index) {
  counterStates[index] = Run;
}

void stopCounter(short index) {
  counterStates[index] = Stop;
}

void stopAllCounter() {
  for (int i = 0; i < (sizeof(counterStates) / sizeof(counterStates[0])); i++) {
    counterStates[i] = Stop;
  }
}

void countUp() {
  for (int i = 0; i < (sizeof(counterStates) / sizeof(counterStates[0])); i++) {
    if (counterStates[i] == Run) {
      counterSeconds[i]++;
    }
  }
}
