enum ScheduleState {
  Future, Present, Past
};

bool isNotifyLedOn = false;

time_t schedules[] = {0, 0, 0, 0, 0};
ScheduleState scheduleStates[] = {Past, Past, Past, Past, Past};
unsigned int scheduleTimeSeconds[] = {0, 0, 0, 0, 0}; // 1 second

void checkScheduleTime() {
  for (int i = 0; i < (sizeof(scheduleStates) / sizeof(scheduleStates[0])); i++) {
    if (scheduleStates[i] != Future) { continue; }
    if (schedules[i] - time(nullptr) > 0) { continue; }
    scheduleStates[i] = Present;
  }
}
