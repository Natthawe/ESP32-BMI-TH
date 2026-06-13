#include "google_sheet.h"

#include <WiFi.h>
#include <HTTPClient.h>

#include "src/core/config.h"
#include "src/core/globals.h"
#include "src/ui/pages/ui_page_bmi.h"
#include "src/services/wifi_manager.h"


bool sendToGoogleSheet(float weightKg, float distanceCm, float heightCm, float bmi, const char* statusText)
{
  if (!uploadToGoogleSheet)
  {
    lastSendStatusText = "LOCAL_ONLY";
    // drawTftScreen(true);
    return true;
  }

  lastSendStatusText = "SENDING";
  // drawTftScreen(true);

  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED)
    {
      lastSendStatusText = "FAILED";
      // drawTftScreen(true);
      return false;
    }
  }

  HTTPClient http;
  http.begin(GOOGLE_SCRIPT_URL);
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"weight_kg\":" + String(weightKg, 3) +
                ",\"distance_cm\":" + String(distanceCm, 1) +
                ",\"height_cm\":" + String(heightCm, 1) +
                ",\"bmi\":" + String(bmi, 2) +
                ",\"status\":\"" + String(statusText) + "\"}";

  int httpCode = http.POST(json);
  String response = (httpCode > 0) ? http.getString() : "";
  http.end();

  if ((httpCode == 200 && response.indexOf("success") >= 0) || httpCode == 302)
  {
    lastSendStatusText = "SUCCESS";
    // drawTftScreen(true);
    return true;
  }

  lastSendStatusText = "FAILED";
  // drawTftScreen(true);
  return false;
}
