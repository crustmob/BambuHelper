#include "display_dual.h"
#include "display_gauges.h"
#include "config.h"
#include "settings.h"
#include "bambu_state.h"

extern TFT_eSPI tft;

// Draw a single printer status in half-screen mode
// x_offset: 0 for left printer, 160 for right printer
// width: 160 pixels per printer
static void drawHalfPrinter(TFT_eSPI& tft, int16_t x_offset, const BambuState& state, const char* name) {
  const int16_t half_w = 160;
  const int16_t cx = x_offset + half_w / 2;  // Center of this half
  
  // Header with printer name/number
  tft.fillRect(x_offset, 0, half_w, 24, CLR_HEADER);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(CLR_TEXT);
  tft.setTextFont(2);
  
  if (state.connected) {
    tft.drawString(name[0] ? name : "Printer", cx, 4);
  } else {
    tft.setTextColor(CLR_RED);
    tft.drawString("Offline", cx, 4);
  }
  
  // Connection indicator dot
  tft.fillCircle(x_offset + 8, 12, 4, state.connected ? CLR_GREEN : CLR_RED);
  
  // Progress bar (H2 style) - scaled down
  if (state.connected) {
    uint8_t progress = state.printProgress;
    const int16_t bar_w = half_w - 20;
    const int16_t bar_h = 4;
    const int16_t bar_x = x_offset + 10;
    const int16_t bar_y = 28;
    
    tft.fillRect(bar_x, bar_y, bar_w, bar_h, CLR_BG);
    
    if (progress > 0) {
      int16_t fill_w = (progress * bar_w) / 100;
      if (fill_w < 1) fill_w = 1;
      tft.fillRoundRect(bar_x, bar_y, fill_w, bar_h, 2, dispSettings.progress.arc);
    }
    
    // Progress percentage
    tft.setTextFont(2);
    tft.setTextColor(CLR_TEXT);
    tft.setTextDatum(MC_DATUM);
    char pct_buf[8];
    snprintf(pct_buf, sizeof(pct_buf), "%d%%", progress);
    tft.drawString(pct_buf, cx, 48);
  }
  
  // Temperature gauges (nozzle and bed) - smaller version
  if (state.connected) {
    // Nozzle temp (left side)
    tft.setTextDatum(RIGHT_DATUM);
    tft.setTextColor(CLR_ORANGE);
    tft.setTextFont(2);
    char nozzle_buf[16];
    snprintf(nozzle_buf, sizeof(nozzle_buf), "%d°C", state.nozzleTemp);
    tft.drawString(nozzle_buf, cx - 10, 75);
    
    // Bed temp (right side)
    tft.setTextDatum(LEFT_DATUM);
    tft.setTextColor(CLR_CYAN);
    char bed_buf[16];
    snprintf(bed_buf, sizeof(bed_buf), "%d°C", state.bedTemp);
    tft.drawString(bed_buf, cx + 10, 75);
    
    // Icons (simple circles for now)
    tft.fillCircle(cx - 35, 75, 6, CLR_ORANGE);  // Nozzle icon
    tft.fillCircle(cx + 35, 75, 6, CLR_CYAN);   // Bed icon
  }
  
  // Time remaining
  if (state.connected && state.printing) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(CLR_TEXT_DIM);
    tft.setTextFont(2);
    tft.drawString("Remaining:", cx, 100);
    
    tft.setTextColor(CLR_TEXT);
    tft.setTextFont(4);
    char time_buf[16];
    if (state.remaining_min > 60) {
      snprintf(time_buf, sizeof(time_buf), "%dh %dm", 
               state.remaining_min / 60, state.remaining_min % 60);
    } else {
      snprintf(time_buf, sizeof(time_buf), "%d min", state.remaining_min);
    }
    tft.drawString(time_buf, cx, 120);
  }
  
  // Speed level
  if (state.connected) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(CLR_TEXT_DIM);
    tft.setTextFont(2);
    tft.drawString("Speed:", cx, 150);
    
    const char* speed_name;
    uint16_t speed_color;
    switch (state.speedLevel) {
      case 1: speed_name = "Silent"; speed_color = CLR_BLUE; break;
      case 2: speed_name = "Std"; speed_color = CLR_GREEN; break;
      case 3: speed_name = "Sport"; speed_color = CLR_ORANGE; break;
      case 4: speed_name = "Ludicr"; speed_color = CLR_RED; break;
      default: speed_name = "---"; speed_color = CLR_TEXT_DIM;
    }
    
    tft.setTextColor(speed_color);
    tft.setTextFont(2);
    tft.drawString(speed_name, cx, 168);
  }
  
  // Divider line on right edge (except for rightmost printer)
  tft.drawFastVLine(x_offset + half_w - 1, 0, SCREEN_H, CLR_HEADER);
}

// Draw dual printer view - both printers side by side
void drawDualPrinterView(TFT_eSPI& tft) {
  // Get both printer states
  BambuState& s1 = printers[0].state;
  BambuState& s2 = printers[1].state;
  const char* n1 = printers[0].config.name;
  const char* n2 = printers[1].config.name;
  
  // Clear screen
  tft.fillScreen(CLR_BG);
  
  // Draw left printer (index 0)
  drawHalfPrinter(tft, 0, s1, n1);
  
  // Draw right printer (index 1)
  drawHalfPrinter(tft, 160, s2, n2);
  
  // Bottom status bar
  tft.fillRect(0, SCREEN_H - 20, SCREEN_W, 20, CLR_HEADER);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(CLR_TEXT_DIM);
  tft.setTextFont(1);
  tft.drawString("BambuHelper Dual View", 4, SCREEN_H - 10);
}

// Update dual view (called from main loop)
void updateDualView() {
  static unsigned long last_update = 0;
  
  if (millis() - last_update > DISPLAY_UPDATE_MS) {
    drawDualPrinterView(tft);
    last_update = millis();
  }
}
