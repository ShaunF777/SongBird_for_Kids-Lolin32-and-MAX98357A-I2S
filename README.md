# 🎵 SongBird for Kids (Lolin32 + MAX98357A I2S)

A standalone, battery-powered "singing bird" — an alarm/music box that checks
internet time and plays an MP3 at a scheduled time. Built as a clean
educational electronics project for an 11-year-old.

## What it does (and will do)

- **Time-based playback**: checks the current time over NTP and plays a
  stored MP3 at a configured alarm hour/minute.
- **Wireless song upload**: a phone-facing web portal lets you upload a new
  song (overwriting the single stored `/song.mp3`) and adjust volume/loop
  settings — no cable needed for day-to-day use.
- **USB-only firmware updates**: OTA is intentionally disabled. Code changes
  are always flashed over USB, keeping the wireless attack surface small and
  the behavior predictable.
- **Battery-friendly**: the board spends most of its life in deep sleep,
  waking only on an RTC timer to check the time and evaluate the alarm, then
  goes back to sleep. WiFi is only brought up when actually needed (time
  sync or serving the web portal).
- **Settings survive power loss**: alarm time, loop count, and volume are
  stored in flash via `Preferences`, so they persist across battery changes.

See `CLAUDE.md` for the full technical specification (not committed to this
repository — see [Repo housekeeping](#repo-housekeeping) below).

## Hardware used

| Part | Notes |
|---|---|
| **Lolin32** (ESP32 dev board) | 4MB flash, **no PSRAM**, no physical BOOT button |
| **MAX98357A** | I2S mono Class-D amplifier breakout |
| **Speaker** | 8Ω, originally a 0.5W speaker for wiring tests; a larger speaker is used for normal listening volume |

### Wiring

| MAX98357A pin | ESP32 pin | Notes |
|---|---|---|
| V | 3.3V | Keeps output power safely limited for small speakers |
| G | GND | |
| BCLK | GPIO 14 | Bit clock — **must be exactly GPIO14**, see [Known issues](#known-issues--fixes) |
| LRCK | GPIO 25 | Left/right (word select) clock |
| DIN | GPIO 26 | Audio data, ESP32 → amplifier |

## Quick start (flashing)

Windows PowerShell, from the project root:

```powershell
chcp 65001                     # avoid a console-encoding bug (see below)
pio run -t uploadfs            # only needed when data/song.mp3 changes
pio run -t upload -t monitor   # flashes firmware, then opens Serial Monitor
```

This board has no BOOT button, so for **both** of the `pio run -t ...`
commands above: short **GPIO0 to GND**, press **RST**, then release GPIO0
once the terminal shows `Connecting.....` succeed (you'll see "Chip type:"
printed). You don't need to hold it through the whole flash — just through
the initial handshake.

`Test/Test_10000reasonsmp3.cpp` has this same cheat-sheet in its header
comment for quick reference while wiring-testing.

## Known issues & fixes

Hard-won debugging notes from getting the audio pipeline working on this
specific board — worth reading before touching `PlatformIO.ini` or the audio
code.

1. **No PSRAM → OOM on newer `ESP32-audioI2S` versions.**
   The library's `lib_deps` git URL originally tracked `master` unpinned.
   Versions from `3.0.0` through the current `master`/`4.0.0` progressively
   require PSRAM — recent versions unconditionally allocate a ~720KB audio
   buffer and hard-fail with `PSRAM not found` on boards without it. This
   board has no PSRAM, so uploads would OOM (`failed to allocate 720896
   bytes for AudioBuffer`) and crash-loop. **Fix:** `PlatformIO.ini` pins
   `lib_deps` to `https://github.com/schreibfaul1/ESP32-audioI2S.git#3.0.0`,
   which auto-falls-back to an 8KB (`1600 * 5`) internal-RAM buffer when no
   PSRAM is found, using the exact same public API (`setPinout`,
   `setVolume`, `connecttoFS`, `loop`, `audio_info`, `audio_eof_stream`).

2. **No physical BOOT button.** Entering bootloader/flash mode requires
   manually shorting GPIO0 to GND and pressing RST at the right moment
   during a `pio run -t upload`/`uploadfs`. There's no way to automate this
   part — it's a physical action.

3. **RST doesn't reliably reboot the running app.** Unplugging and
   replugging USB works but breaks any already-open `pio device monitor`
   session (a stale serial handle doesn't reconnect to the new USB
   enumeration). **Fix:** don't unplug at all — just close and reopen `pio
   device monitor` (or run `pio run -t upload -t monitor`). Opening a
   *fresh* serial connection toggles the DTR/RTS lines by default, which
   resets the ESP32 the same way esptool's own "Hard resetting via RTS
   pin..." does after a normal upload.

4. **Windows console encoding bug.** `pio run -t upload` can appear to hang
   indefinitely mid-flash. The real cause: PlatformIO's progress bar writes
   UTF-8 block-drawing characters that crash on the default Windows cp1252
   console (`UnicodeEncodeError` inside a background echo thread), silently
   stalling all further output without actually killing the upload. **Fix:**
   run `chcp 65001` (and optionally `$env:PYTHONIOENCODING = "utf-8"`)
   before any `pio` command.

5. **`audio.loop()` must be called with no delay.** It needs to be fed
   continuously, many times per millisecond, to keep the I2S buffer full.
   Any `delay()` placed around/before it in the Arduino `loop()` starves the
   buffer almost immediately and effectively kills playback.

6. **Song files flash separately from firmware.** `data/song.mp3` (source
   copied in as `/song.mp3`) is written to the LittleFS partition via `pio
   run -t uploadfs`, entirely independent from `pio run -t upload`, which
   only flashes the compiled firmware. Forgetting to re-run `uploadfs` after
   changing the song is a common source of "nothing changed" confusion.

7. **Wiring precision matters.** BCLK must land on exactly GPIO14 as wired
   in the code (`I2S_BCLK`). A miswire to GPIO13 produced completely silent
   playback with **no error logged** — the decoder ran fine, it just never
   reached the speaker. If audio_info logs show a clean `stream ready` →
   `End of file` sequence but you still hear nothing, check wiring before
   suspecting the code.

## Repo housekeeping

`.gitignore` deliberately excludes:
- `.claude/` — Claude Code's local session/settings files, not project code
- `CLAUDE.md` — the internal spec/instructions file for AI-assisted
  development, not meant to be published
- `*.mp3` — song files are personal media, not source code, and can be
  large; they're uploaded to the board separately via `pio run -t uploadfs`
  (see [Quick start](#quick-start-flashing))
