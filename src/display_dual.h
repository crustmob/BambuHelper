#ifndef DISPLAY_DUAL_H
#define DISPLAY_DUAL_H

#include <TFT_eSPI.h>
#include "bambu_state.h"

void drawDualPrinterView(TFT_eSPI& tft);
void updateDualView();

#endif // DISPLAY_DUAL_H
