#include "AudioModule.h"
#include "Config.h"
#include <LittleFS.h>
#include "Audio.h"

// The Audio library owns a fair bit of internal state, so we keep one
// instance for the whole program's lifetime, private to this file.
static Audio audio;

namespace AudioModule {

void begin() {
  audio.setPinout(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DOUT);
  audio.setVolume(DEFAULT_VOLUME);
}

void playSong(const char* path) {
  audio.connecttoFS(LittleFS, path);
}

void update() {
  audio.loop();
}

bool isPlaying() {
  return audio.isRunning();
}

void setVolume(uint8_t vol) {
  audio.setVolume(vol);
}

}  // namespace AudioModule

// ---------------------------------------------------------
// The ESP32-audioI2S library calls these two functions by
// name (not through a class), so they must be free functions
// with exactly this signature, living somewhere in the build.
// ---------------------------------------------------------
void audio_info(const char* info) {
  Serial.print("audio_info: ");
  Serial.println(info);
}

void audio_eof_stream(const char* info) {
  Serial.print("Done playing: ");
  Serial.println(info);
}
