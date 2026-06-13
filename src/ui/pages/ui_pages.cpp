#include "ui_pages.h"

#include "src/core/globals.h"
#include "src/core/states.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/pages/ui_page_main.h"
#include "src/ui/pages/ui_page_bmi.h"
#include "src/ui/components/ui_keyboard.h"
#include "src/ui/pages/ui_page_wifi.h"
#include "src/ui/pages/ui_page_thai_test.h"


void drawUiScreen(bool force)
{
  if (!force && !tftFullRedrawRequested && currentUiPage == previousUiPage) return;

  previousUiPage = currentUiPage;

  switch (currentUiPage)
  {
    case UI_THAI_TEST:        drawPageThaiTest(); break;
    case UI_SENSOR_CHECK:     drawPageSensorCheck(); break;
    case UI_SENSOR_ERROR:     drawPageSensorError(); break;
    case UI_MAIN_MENU:        drawPageMainMenu(); break;
    case UI_WIFI_SETTINGS:    drawPageWifiSettings(); break;
    case UI_WIFI_CONNECTING:  drawPageWifiConnectingUi(); break;
    case UI_WIFI_SUCCESS:     drawPageWifiSuccessUi(); break;
    case UI_WIFI_FAILED:      drawPageWifiFailedUi(); break;
    case UI_EDIT_SSID:        drawPageKeyboard(); break;
    case UI_EDIT_PASSWORD:    drawPageKeyboard(); break;
    case UI_BMI_MEASURE:      drawTftScreen(force); break;
    default:                  break;
  }
}
