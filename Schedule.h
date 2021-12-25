enum ScheduleState {
  Future, Present, Past
};

bool isNotifyLedOn = false;

time_t schedules[] = {0, 0, 0, 0, 0};
ScheduleState scheduleStates[] = {Past, Past, Past, Past, Past};
unsigned int scheduleTimeSeconds[] = {0, 0, 0, 0, 0}; // 1 second

int remainSecondsDefaultValue = 2147483647;  // 32bit int max
int remainSecondsForDisplay = remainSecondsDefaultValue;

void checkScheduleTime() {
  for (int i = 0; i < (sizeof(scheduleStates) / sizeof(scheduleStates[0])); i++) {
    if (scheduleStates[i] == Past) { continue; }

    int remainSeconds = schedules[i] - time(nullptr);
    if (remainSecondsForDisplay >= remainSeconds) {
      remainSecondsForDisplay = remainSeconds;
    }

    if (scheduleStates[i] == Present) { continue; }
    if (schedules[i] - time(nullptr) > 0) { continue; }  // Future State

    scheduleStates[i] = Present;
  }
}
