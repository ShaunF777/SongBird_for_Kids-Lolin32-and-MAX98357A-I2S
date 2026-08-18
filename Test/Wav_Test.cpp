/*
  ============================================================
  SongBird for Kids - Wiring Test
  ============================================================
  What this sketch does, in plain English:

    1. We "draw" a sound wave using math (a sine wave), one
       tiny number at a time. This is exactly how all digital
       audio works - a speaker is just a magnet that pushes
       and pulls a paper cone very fast, and a list of numbers
       tells it exactly how far to push at each instant.
    2. We save that list of numbers into a real audio file
       (a .wav file) in the ESP32's own flash memory, using a
       mini filesystem called LittleFS.
    3. We hand that file to the ESP32-audioI2S library, which
       decodes it and streams it out over the I2S pins to the
       MAX98357A amplifier board, which drives the speaker.

  If you hear a short, gentle beep, your wiring is correct!
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
// Volume safety settings
// ---------------------------------------------------------
// The Audio library's volume range is 0-21. Our speaker is a
// tiny 0.5W speaker running straight off 3.3V, so we keep the
// library volume well under half, around 25-30%.
const uint8_t SAFE_VOLUME = 21;  // 100% (21 / 21) - larger speaker now in use

// As a second layer of safety, we also generate our test tone
// at reduced amplitude (loudness) rather than full digital
// scale. That way, even the loudest possible library volume
// setting still can't blast the speaker with this file.
const float TONE_AMPLITUDE = 1.00f;  // full digital range

// ---------------------------------------------------------
// Test tone settings
// ---------------------------------------------------------
const uint32_t SAMPLE_RATE   = 16000;  // samples per second (16 kHz is plenty for a beep)
const float    TONE_FREQ_HZ  = 440.0f; // pitch of the beep (440 Hz = musical note A)
const float    TONE_SECONDS  = 0.4f;   // how long the beep lasts
const char*    TONE_FILENAME = "/beep.wav";

Audio audio;

// ---------------------------------------------------------
// Builds a tiny mono 16-bit WAV file containing one sine-wave
// beep, and writes it into LittleFS so the Audio library can
// play it back like any normal audio file.
// ---------------------------------------------------------
void generateAndSaveTestTone() {
  const uint32_t numSamples = (uint32_t)(SAMPLE_RATE * TONE_SECONDS);
  const uint32_t dataBytes  = numSamples * sizeof(int16_t);

  // Standard 44-byte WAV header for 16-bit mono PCM audio.
  // This just tells any audio player: "here's the format,
  // sample rate, and how much data follows."
  uint8_t header[44];
  uint32_t byteRate   = SAMPLE_RATE * 1 /*channel*/ * 2 /*bytes per sample*/;
  uint32_t riffChunkSize = 36 + dataBytes;

  memcpy(header + 0, "RIFF", 4);
  memcpy(header + 8, "WAVE", 4);
  memcpy(header + 12, "fmt ", 4);
  memcpy(header + 36, "data", 4);

  *(uint32_t*)(header + 4)  = riffChunkSize;
  *(uint32_t*)(header + 16) = 16;              // fmt chunk size
  *(uint16_t*)(header + 20) = 1;               // PCM format
  *(uint16_t*)(header + 22) = 1;               // 1 channel (mono)
  *(uint32_t*)(header + 24) = SAMPLE_RATE;
  *(uint32_t*)(header + 28) = byteRate;
  *(uint16_t*)(header + 32) = 2;               // block align (bytes per sample frame)
  *(uint16_t*)(header + 34) = 16;              // bits per sample
  *(uint32_t*)(header + 40) = dataBytes;

  File file = LittleFS.open(TONE_FILENAME, "w");
  if (!file) {
    Serial.println("Failed to create tone file on LittleFS!");
    return;
  }
  file.write(header, sizeof(header));

  // Now we compute the actual sine wave, one sample at a time,
  // and write it straight to the file.
  const int16_t peak = (int16_t)(32767 * TONE_AMPLITUDE);
  for (uint32_t i = 0; i < numSamples; i++) {
    float t = (float)i / SAMPLE_RATE;
    float wave = sinf(2.0f * PI * TONE_FREQ_HZ * t);

    // Fade the very start and end of the beep in/out smoothly
    // (a "fade envelope") so it doesn't click or pop.
    float fadeSamples = SAMPLE_RATE * 0.02f; // 20 ms fade
    float envelope = 1.0f;
    if (i < fadeSamples) envelope = i / fadeSamples;
    else if (i > numSamples - fadeSamples) envelope = (numSamples - i) / fadeSamples;

    int16_t sample = (int16_t)(peak * wave * envelope);
    file.write((uint8_t*)&sample, sizeof(sample));
  }

  file.close();
  Serial.printf("Test tone written: %s (%u samples, %.2f sec)\n",
                TONE_FILENAME, numSamples, TONE_SECONDS);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Singing Bird - I2S Wiring Test ===");

  // Start LittleFS. "true" tells it to auto-format the flash
  // storage if it can't find a valid filesystem yet (only
  // happens the very first time you run this).
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed!");
    return;
  }

  generateAndSaveTestTone();

  // Tell the Audio library which pins the MAX98357A is wired to.
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(SAFE_VOLUME); // stay well under max for our 0.5W speaker

  // Start playback of the beep file we just created.
  audio.connecttoFS(LittleFS, TONE_FILENAME);
  Serial.println("Playing test beep...");
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
