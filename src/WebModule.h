/*
  ============================================================
  WebModule.h - serves the phone-facing settings/upload portal
  ============================================================
  Runs *after* WifiModule's credential portal has already
  connected to Wi-Fi and shut itself down - this module starts
  its own ESPAsyncWebServer instance, so the two never overlap.
  ============================================================
*/

#pragma once

#include "Config.h"

namespace WebModule {

// Starts the web server. `settings` is shown as the current
// values in the settings form, and is updated in place whenever
// the user saves changes or uploads a new song. main.cpp still
// owns saving the updated settings to Preferences afterward.
void begin(Settings& settings);

// Stops the web server. Call before deep sleep.
void end();

}  // namespace WebModule
