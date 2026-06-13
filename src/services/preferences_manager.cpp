#include "preferences_manager.h"

#include <Preferences.h>
#include "src/core/config.h"
#include "src/core/globals.h"


Preferences prefs;
bool prefsReady = false;

void loadWifiPreferences()
{
  prefsReady = prefs.begin(PREF_NAMESPACE, false);

  if (!prefsReady)
  {
    Serial.println("Preferences begin failed, using default WiFi credentials");
    return;
  }

  String savedSsid = prefs.getString(PREF_KEY_SSID, wifiSsid);
  String savedPass = prefs.getString(PREF_KEY_PASS, wifiPassword);

  if (savedSsid.length() > 0)
  {
    wifiSsid = savedSsid;
  }

  // Password อนุญาตให้เป็นค่าว่างได้ กรณี WiFi ไม่มีรหัสผ่าน
  wifiPassword = savedPass;

  Serial.print("Loaded WiFi SSID from Preferences: ");
  Serial.println(wifiSsid);
}

void saveWifiPreferences()
{
  if (!prefsReady)
  {
    Serial.println("Preferences not ready, cannot save WiFi credentials");
    return;
  }

  prefs.putString(PREF_KEY_SSID, wifiSsid);
  prefs.putString(PREF_KEY_PASS, wifiPassword);

  Serial.print("Saved WiFi SSID to Preferences: ");
  Serial.println(wifiSsid);
}
