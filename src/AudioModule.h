/*
  ============================================================
  AudioModule.h - talks directly to the MAX98357A speaker
  ============================================================
  Wraps the ESP32-audioI2S library so the rest of the code
  never has to touch the Audio object directly.
  ============================================================
*/

#pragma once

#include <Arduino.h>

namespace AudioModule {

// Sets the I2S pins and starting volume. Call once from setup().
void begin();

// Starts playing an MP3 file already stored in LittleFS.
void playSong(const char* path);

// Must be called every loop() iteration, with no delay()
// around it, so the audio buffer stays full.
void update();

// True while a song is actively playing.
bool isPlaying();

// vol is 0-21, matching the Audio library's own range.
void setVolume(uint8_t vol);

}  // namespace AudioModule
