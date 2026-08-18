/*
  ============================================================
  SongBird for Kids - main orchestrator
  ============================================================
  Every time the board wakes up (from deep sleep, or from a
  fresh power-on/reset) it runs this whole sequence once, then
  goes back to sleep until the next scheduled wake-up:

    1. Load saved settings (alarm time, loop count, volume).
    2. Connect to Wi-Fi (or open a setup portal on first use).
    3. Sync the clock over NTP.
    4. Open the settings/upload web portal for a short window,
       so a phone can change settings or upload a new song.
    5. Save any changes made through the portal.
    6. If the current time matches the alarm, play the song.
    7. Work out how long until the next alarm, and deep-sleep
       for exactly that long.

  Deep sleep restarts the chip back at setup() - there's no
  need for anything in loop().
  ============================================================
*/

#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>
#include "Config.h"
#include "AudioModule.h"
#include "WifiModule.h"
#include "TimeModule.h"
#include "WebModule.h"

static void playAlarmSong(const Settings& settings) {
  AudioModule::setVolume(settings.volume);

  Serial.print("Alarm time! Playing song ");
  Serial.print(settings.loopCount);
  Serial.println(" time(s)...");

  for (int i = 0; i < settings.loopCount; i++) {
    AudioModule::playSong(SONG_FILENAME);
    while (AudioModule::isPlaying()) {
      AudioModule::update();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== SongBird for Kids ===");

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed!");
    return;
  }

  Preferences prefs;
  prefs.begin(PREFS_NAMESPACE, false);

  Settings settings;
  settings.alarmHour   = prefs.getInt(PREFS_ALARM_HOUR, DEFAULT_ALARM_HOUR);
  settings.alarmMinute = prefs.getInt(PREFS_ALARM_MIN, DEFAULT_ALARM_MINUTE);
  settings.loopCount   = prefs.getInt(PREFS_LOOP_COUNT, DEFAULT_LOOP_COUNT);
  settings.volume      = prefs.getInt(PREFS_VOLUME, DEFAULT_VOLUME);

  AudioModule::begin();
  AudioModule::setVolume(settings.volume);

  bool wifiOk = WifiModule::connect();

  if (wifiOk) {
    TimeModule::sync();

    WebModule::begin(settings);
    Serial.print("WebModule: portal open for ");
    Serial.print(WEB_PORTAL_WINDOW_SECONDS);
    Serial.println(" seconds (use the Test Play button to check audio)...");
    unsigned long portalStart = millis();
    while (millis() - portalStart < WEB_PORTAL_WINDOW_SECONDS * 1000UL) {
      AudioModule::update();
      // Only skip the yield while a song is actively streaming - idling in a
      // truly tight loop for up to 120 seconds starves the core's idle task
      // and trips the Task Watchdog Timer, silently rebooting the board.
      if (!AudioModule::isPlaying()) {
        delay(1);
      }
    }
    WebModule::end();

    // Save whatever the portal left settings as - a no-op write if nothing changed.
    prefs.putInt(PREFS_ALARM_HOUR, settings.alarmHour);
    prefs.putInt(PREFS_ALARM_MIN, settings.alarmMinute);
    prefs.putInt(PREFS_LOOP_COUNT, settings.loopCount);
    prefs.putInt(PREFS_VOLUME, settings.volume);

    int hour, minute;
    if (TimeModule::getCurrentTime(hour, minute)) {
      Serial.print("Current time: ");
      Serial.print(hour);
      Serial.print(":");
      Serial.println(minute);

      if (hour == settings.alarmHour && minute == settings.alarmMinute) {
        playAlarmSong(settings);
      }
    }
  } else {
    Serial.println("No Wi-Fi this cycle - skipping time sync, portal, and alarm check.");
  }

  uint64_t sleepSeconds = TimeModule::secondsUntil(settings.alarmHour, settings.alarmMinute);
  prefs.end();
  WifiModule::disconnect();

  Serial.print("Going to sleep for ");
  Serial.print((unsigned long)sleepSeconds);
  Serial.println(" seconds.");
  Serial.flush();

  ESP.deepSleep(sleepSeconds * 1000000ULL);
}

void loop() {
  // Never reached - deep sleep restarts the chip at setup().
}
