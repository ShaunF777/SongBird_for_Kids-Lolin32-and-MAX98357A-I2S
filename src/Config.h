/*
  ============================================================
  Config.h - the blueprint configuration
  ============================================================
  All pin numbers and tunable settings live here, in one place,
  so nothing is buried inside a module file.
  ============================================================
*/

#pragma once

#include <Arduino.h>

// ---------------------------------------------------------
// I2S pin wiring (matches MAX98357A -> ESP32 connections)
// ---------------------------------------------------------
#define PIN_I2S_BCLK  14   // Bit clock
#define PIN_I2S_LRC   25   // Left/Right (word select) clock
#define PIN_I2S_DOUT  26   // Audio data out (ESP32 -> amplifier)

// ---------------------------------------------------------
// Audio settings
// ---------------------------------------------------------
// The Audio library's volume range is 0-21.
const uint8_t DEFAULT_VOLUME = 6;  // ~30% of the library's 0-21 range
const char* const SONG_FILENAME = "/song.mp3";

// ---------------------------------------------------------
// Wi-Fi portal settings
// ---------------------------------------------------------
const char* const WIFI_PORTAL_AP_NAME = "SongBird Setup";
const unsigned long WIFI_CONNECT_TIMEOUT_SECONDS = 15;   // trying saved credentials
const unsigned long WIFI_PORTAL_TIMEOUT_SECONDS  = 180;  // waiting in the captive portal

// ---------------------------------------------------------
// Alarm defaults (used the very first time, before any
// settings have been saved via the web portal)
// ---------------------------------------------------------
const int DEFAULT_ALARM_HOUR   = 7;
const int DEFAULT_ALARM_MINUTE = 0;
const int DEFAULT_LOOP_COUNT   = 1;

// ---------------------------------------------------------
// Persistent storage (Preferences) namespace + keys
// ---------------------------------------------------------
const char* const PREFS_NAMESPACE   = "songbird";
const char* const PREFS_ALARM_HOUR  = "alarm_hour";
const char* const PREFS_ALARM_MIN   = "alarm_minute";
const char* const PREFS_LOOP_COUNT  = "loop_count";
const char* const PREFS_VOLUME      = "volume_level";

// ---------------------------------------------------------
// NTP time settings
// ---------------------------------------------------------
const char* const NTP_SERVER = "pool.ntp.org";
// South Africa (SAST, UTC+2) - no daylight savings observed.
const long GMT_OFFSET_SEC       = 2 * 3600;
const int  DAYLIGHT_OFFSET_SEC  = 0;

// ---------------------------------------------------------
// How long the settings/upload web portal stays open on each
// wake cycle, before the board goes back to sleep.
// ---------------------------------------------------------
const unsigned long WEB_PORTAL_WINDOW_SECONDS = 300;  // 5 min while testing; tighten up later

// ---------------------------------------------------------
// The settings a child (or parent) can change from the web
// portal. Loaded from Preferences at boot, saved back to
// Preferences whenever the portal saves changes.
// ---------------------------------------------------------
struct Settings {
  int alarmHour   = DEFAULT_ALARM_HOUR;
  int alarmMinute = DEFAULT_ALARM_MINUTE;
  int loopCount   = DEFAULT_LOOP_COUNT;
  int volume      = DEFAULT_VOLUME;
};
