/*
  ============================================================
  TimeModule.h - coordinates NTP time checks
  ============================================================
  Wraps the ESP32's built-in time functions so the rest of the
  code can just ask "what time is it?" and "how long until the
  alarm?".
  ============================================================
*/

#pragma once

#include <Arduino.h>

namespace TimeModule {

// Asks an NTP server for the current time and sets the ESP32's
// internal clock. Call this once, after Wi-Fi is connected.
// Returns true if the time was received successfully.
bool sync();

// Fills in the current local hour (0-23) and minute (0-59).
// Returns false if the clock isn't set yet (e.g. sync() was
// never called, or it failed and this is a fresh boot).
bool getCurrentTime(int& hour, int& minute);

// How many seconds from right now until the given alarm
// hour/minute next happens - later today if it hasn't happened
// yet, otherwise tomorrow. Used to schedule the next deep-sleep
// wake-up. Falls back to a plain 24-hour wait if the clock isn't
// set (better to check again tomorrow than to guess a time).
uint64_t secondsUntil(int alarmHour, int alarmMinute);

}  // namespace TimeModule
