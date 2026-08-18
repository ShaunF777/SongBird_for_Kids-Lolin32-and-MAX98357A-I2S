#include "WifiModule.h"
#include "Config.h"
#include <Arduino.h>
#include <WiFiManager.h>

namespace WifiModule {

bool connect() {
  WiFiManager wm;
  wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT_SECONDS);
  wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_SECONDS);

  bool connected = wm.autoConnect(WIFI_PORTAL_AP_NAME);

  if (!connected) {
    Serial.println("WiFi: could not connect (portal timed out or was cancelled).");
    return false;
  }

  Serial.print("WiFi: connected, IP = ");
  Serial.println(WiFi.localIP());
  return true;
}

void disconnect() {
  WiFi.disconnect(true, true);  // drop connection and erase the radio's cached state
  WiFi.mode(WIFI_OFF);
}

bool isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

}  // namespace WifiModule
