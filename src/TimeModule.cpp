#include "TimeModule.h"
#include "Config.h"
#include <time.h>

namespace TimeModule {

bool sync() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  struct tm timeInfo;
  if (!getLocalTime(&timeInfo, 10000)) {  // give the NTP server up to 10 seconds
    Serial.println("TimeModule: NTP sync failed.");
    return false;
  }

  Serial.print("TimeModule: synced, current time is ");
  Serial.println(&timeInfo, "%H:%M:%S");
  return true;
}

bool getCurrentTime(int& hour, int& minute) {
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo, 100)) {  // clock is already set, so this should be instant
    return false;
  }
  hour = timeInfo.tm_hour;
  minute = timeInfo.tm_min;
  return true;
}

uint64_t secondsUntil(int alarmHour, int alarmMinute) {
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo, 100)) {
    Serial.println("TimeModule: clock not set, retrying in 24 hours.");
    return 24UL * 60 * 60;
  }

  int nowSeconds = timeInfo.tm_hour * 3600 + timeInfo.tm_min * 60 + timeInfo.tm_sec;
  int alarmSeconds = alarmHour * 3600 + alarmMinute * 60;

  int diff = alarmSeconds - nowSeconds;
  if (diff <= 0) {
    diff += 24 * 60 * 60;  // alarm time already passed today - wait for tomorrow
  }
  return (uint64_t)diff;
}

}  // namespace TimeModule
