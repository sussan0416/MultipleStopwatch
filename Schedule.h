enum ScheduleState {
  Future, Present, Past
};

bool isNotifyLedOn = false;

time_t schedules[] = {0, 0, 0, 0, 0};
ScheduleState scheduleStates[] = {Past, Past, Past, Past, Past};
unsigned int scheduleTimeSeconds[] = {0, 0, 0, 0, 0}; // 1 second

int remainSecondsDefaultValue = 2147483647;  // 32bit int max
int remainSecondsForDisplay = remainSecondsDefaultValue;
