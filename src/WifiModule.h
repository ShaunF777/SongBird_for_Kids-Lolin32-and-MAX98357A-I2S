/*
  ============================================================
  WifiModule.h - manages connection states and portal loops
  ============================================================
  Wraps WiFiManager so the rest of the code just asks for a
  connection and gets true/false back.
  ============================================================
*/

#pragma once

namespace WifiModule {

// Tries the saved Wi-Fi credentials first; if that fails, opens a
// captive setup portal (see WIFI_PORTAL_AP_NAME in Config.h) so the
// user can pick a network from their phone. Blocks until connected
// or the portal times out. Returns true if Wi-Fi is connected.
bool connect();

// Turns the radio off before deep sleep, to save battery.
void disconnect();

bool isConnected();

}  // namespace WifiModule
