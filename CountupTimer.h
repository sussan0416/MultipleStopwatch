#include <Ticker.h>

enum CounterState {
  Stop, Run
};

enum TaskType {
  Single, Multiple
};

TaskType taskType = Single;

#define TICKER_COUNTUP_SEC 0.1
#define TICKER_PRINT_SEC 0.2

// Tickers
Ticker countUpTicker;
Ticker mainViewTicker;

unsigned int counterSeconds[] = {0, 0, 0, 0, 0}; // 1/10 second
CounterState counterStates[] = {Stop, Stop, Stop, Stop, Stop};
const char buttonChars[] = {'A', 'B', 'C', 'D', 'E'};

short adjustTarget = -1;

void resetCounter(int index) {
  counterSeconds[index] = 0;
  counterStates[index] = Stop;
}

void resetAllCounter() {
  for (int i = 0; i < (sizeof(counterStates) / sizeof(counterStates[0])); i++) {
    counterSeconds[i] = 0;
  }
}

void startCounter(int index) {
  counterStates[index] = Run;
}

void stopCounter(int index) {
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

void getCounterSeconds(int i, char *dest) {
  unsigned int t = counterSeconds[i] / 10;
  sprintf(dest, "%2d:%02d:%02d", t / 60 / 60, t / 60 % 60, t % 60);
}

void getCounterSecondsSum(char *dest) {
  unsigned int sum = 0;
  for (int i = 0; i < (sizeof(counterSeconds) / sizeof(counterSeconds[0])); i++) {
    sum += counterSeconds[i] / 10;
  }
  sprintf(dest, "%d:%02d", sum / 60 / 60, sum / 60 % 60);
}
