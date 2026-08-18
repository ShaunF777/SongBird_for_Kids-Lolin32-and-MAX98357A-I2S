/*
  ============================================================
  SongBird for Kids - MP3 Playback Test
  ============================================================
  What this sketch does, in plain English:

    1. We already copied "10,000 Reasons - Live - Phil Wickham.mp3"
       into the ESP32's flash memory (LittleFS) as "/song.mp3",
       using PlatformIO's "Upload Filesystem Image" step.
    2. This sketch hands that file to the ESP32-audioI2S library,
       which decodes the MP3 and streams it out over the I2S pins
       to the MAX98357A amplifier board, which drives the speaker.

  If you hear the song playing, your wiring and audio pipeline
  are both working correctly!
  ============================================================

  HOW TO FLASH & TEST THIS SKETCH (run these yourself in your
  terminal — Claude can't press the physical GPIO0/RST buttons):

    1. chcp 65001
       (fixes a Windows console bug that can freeze pio mid-upload)

    2. pio run -t uploadfs
       (uploads data/song.mp3 into LittleFS as /song.mp3 — only
       needed again if the song file changes)

    3. pio run -t upload -t monitor
       (uploads this firmware, then opens the Serial Monitor;
       opening a fresh monitor session resets the board via
       RTS/DTR automatically, so you'll catch the full boot log)

    For both (2) and (3): short GPIO0 to GND, then press RST to
    enter bootloader mode, and release GPIO0 once you see
    "Connecting....." succeed in the terminal.
  ============================================================
*/

#include <Arduino.h>
#include <LittleFS.h>
#include "Audio.h"

// ---------------------------------------------------------
// I2S pin wiring (matches MAX98357A -> ESP32 connections)
// ---------------------------------------------------------
#define I2S_BCLK  14   // Bit clock
#define I2S_LRC   25   // Left/Right (word select) clock
#define I2S_DOUT  26   // Audio data out (ESP32 -> amplifier)

// ---------------------------------------------------------
// Volume setting
// ---------------------------------------------------------
// The Audio library's volume range is 0-21.
const uint8_t SAFE_VOLUME = 21;  // 100% (21 / 21) - larger speaker now in use

// ---------------------------------------------------------
// Song file settings
// ---------------------------------------------------------
const char* SONG_FILENAME = "/song.mp3";

Audio audio;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Singing Bird - MP3 Playback Test ===");

  // Start LittleFS. "true" tells it to auto-format the flash
  // storage if it can't find a valid filesystem yet (only
  // happens the very first time you run this).
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed!");
    return;
  }

  // Tell the Audio library which pins the MAX98357A is wired to.
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(SAFE_VOLUME);

  // Start playback of the song file uploaded via "Upload Filesystem Image".
  audio.connecttoFS(LittleFS, SONG_FILENAME);
  Serial.println("Playing song...");
}

void loop() {
  // The Audio library needs to be "fed" continuously so it can
  // keep streaming decoded audio out over I2S. Nothing else in
  // this sketch needs to happen in the main loop.
  audio.loop();
}

// ---------------------------------------------------------
// Optional status callback from the Audio library - just
// prints what's happening so you can watch along in the
// Serial Monitor with your daughter.
// ---------------------------------------------------------
void audio_info(const char *info) {
  Serial.print("audio_info: ");
  Serial.println(info);
}

void audio_eof_stream(const char *info) {
  Serial.print("Done playing: ");
  Serial.println(info);
}
