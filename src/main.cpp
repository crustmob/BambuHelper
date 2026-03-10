#include <Arduino.h>
#include "display_ui.h"
#include "display_dual.h"
#include "settings.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "bambu_mqtt.h"
#include "config.h"
#include "bambu_state.h"
#include "touch_input.h"

static unsigned long splashEnd = 0;
static unsigned long finishScreenStart = 0;
static int8_t currentPrinterIndex = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== BambuHelper Starting ===");

  loadSettings();        // load first so rotation/colors are ready
  initDisplay();         // now uses dispSettings.rotation
  initTouch();           // initialize resistive touch
  splashEnd = millis() + 2000;
  setBacklight(brightness);
}

void loop() {
  // Hold splash for 2s
  if (splashEnd > 0 && millis() > splashEnd) {
    splashEnd = 0;
    initWiFi();
    initWebServer();
    initBambuMqtt();
  }

  if (splashEnd > 0) {
    delay(10);
    return;
  }

  handleTouch();         // check for swipe gestures
  
  handleWiFi();
  handleWebServer();

  if (isWiFiConnected() && !isAPMode()) {
    // Check if we have multiple printers configured
    bool dualMode = (printers[0].config.enabled && printers[1].config.enabled);
    
    if (dualMode) {
      // Dual printer mode - connect to both
      handleBambuMqtt();  // This handles all enabled printers
      updateDualView();   // Draw both printers side by side
    } else if (isPrinterConfigured()) {
      handleBambuMqtt();
    }

    // Auto-select screen based on printer state (single printer mode)
    if (!dualMode) {
    BambuState& s = activePrinter().state;
    ScreenState current = getScreenState();

    if (!isPrinterConfigured()) {
      // No printer configured — show idle (user can configure via web)
      if (current != SCREEN_IDLE && current != SCREEN_OFF) {
        setScreenState(SCREEN_IDLE);
        finishScreenStart = 0;
      }
    } else if (!s.connected && current != SCREEN_CONNECTING_MQTT && current != SCREEN_OFF) {
      setScreenState(SCREEN_CONNECTING_MQTT);
      finishScreenStart = 0;
    } else if (!s.connected && current == SCREEN_OFF) {
      // Stay off when printer is disconnected/off
    } else if (s.connected && s.printing) {
      if (current != SCREEN_PRINTING) {
        setScreenState(SCREEN_PRINTING);
        finishScreenStart = 0;
      }
    } else if (s.connected && !s.printing &&
               strcmp(s.gcodeState, "FINISH") == 0) {
      if (current != SCREEN_FINISHED && current != SCREEN_OFF) {
        setScreenState(SCREEN_FINISHED);
        finishScreenStart = millis();
      }
      // Auto-off after finish timeout (unless keepDisplayOn)
      if (current == SCREEN_FINISHED && !dpSettings.keepDisplayOn &&
          dpSettings.finishDisplayMins > 0 && finishScreenStart > 0 &&
          millis() - finishScreenStart > (unsigned long)dpSettings.finishDisplayMins * 60000UL) {
        setScreenState(SCREEN_OFF);
      }
    } else if (s.connected && !s.printing &&
               strcmp(s.gcodeState, "FINISH") != 0) {
      if (current == SCREEN_OFF) {
        // Printer woke up from off state — restore display
        setBacklight(brightness);
      }
      if (current != SCREEN_IDLE) {
        setScreenState(SCREEN_IDLE);
        finishScreenStart = 0;
      }
    }
    }  // end if (!dualMode)
  }

  // Update display (single printer mode only - dual mode updates in its own function)
  bool dualMode = (printers[0].config.enabled && printers[1].config.enabled);
  if (!dualMode) {
    updateDisplay();
  }
}
