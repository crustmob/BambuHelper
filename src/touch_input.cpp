#include "touch_input.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

static XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
static bool touchInitialized = false;

// Swipe detection thresholds
#define SWIPE_THRESHOLD_X  80   // Minimum horizontal distance for swipe
#define SWIPE_THRESHOLD_Y  40   // Maximum vertical deviation for horizontal swipe
#define TAP_THRESHOLD      10   // Max movement for tap detection

static int32_t lastX = -1, lastY = -1;
static uint32_t lastPressTime = 0;
static bool isPressed = false;

void initTouch() {
  if (touchInitialized) return;
  
  ts.begin();
  ts.setRotation(3);  // Landscape rotation
  touchInitialized = true;
  Serial.println("Touch: initialized");
}

bool readTouch(int32_t *x, int32_t *y) {
  if (!touchInitialized) return false;
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    *x = p.x;
    *y = p.y;
    return true;
  }
  return false;
}

TouchGesture readGesture() {
  if (!touchInitialized) return GESTURE_NONE;
  
  int32_t x, y;
  
  // Check if touch is pressed
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    
    if (!isPressed) {
      // New press - record start position
      lastX = p.x;
      lastY = p.y;
      lastPressTime = millis();
      isPressed = true;
    }
    return GESTURE_NONE;  // Still holding
  }
  
  if (isPressed) {
    // Touch released - analyze gesture
    isPressed = false;
    
    int32_t endX = lastX;
    int32_t endY = lastY;
    uint32_t duration = millis() - lastPressTime;
    
    // Calculate delta
    int32_t deltaX = endX - lastX;
    int32_t deltaY = abs(endY - lastY);
    
    // Check for swipe
    if (abs(deltaX) > SWIPE_THRESHOLD_X && deltaY < SWIPE_THRESHOLD_Y) {
      if (deltaX > 0) {
        Serial.println("Touch: SWIPE_RIGHT");
        return GESTURE_SWIPE_RIGHT;
      } else {
        Serial.println("Touch: SWIPE_LEFT");
        return GESTURE_SWIPE_LEFT;
      }
    }
    
    // Check for tap (short press, minimal movement)
    if (duration < 300 && abs(deltaX) < TAP_THRESHOLD && deltaY < TAP_THRESHOLD) {
      Serial.println("Touch: TAP");
      return GESTURE_TAP;
    }
    
    lastX = -1;
    lastY = -1;
  }
  
  return GESTURE_NONE;
}

void handleTouch() {
  TouchGesture gesture = readGesture();
  
  if (gesture != GESTURE_NONE) {
    // Handle gesture in UI
    handleTouchGesture(gesture);
  }
}

void handleTouchGesture(TouchGesture gesture) {
  // Forward to display UI for handling
  // This will be called from main loop
  switch (gesture) {
    case GESTURE_SWIPE_LEFT:
      Serial.println("Gesture: Swipe Left - next printer");
      // TODO: Switch to next printer
      break;
    case GESTURE_SWIPE_RIGHT:
      Serial.println("Gesture: Swipe Right - previous printer");
      // TODO: Switch to previous printer
      break;
    case GESTURE_TAP:
      Serial.println("Gesture: Tap");
      // TODO: Toggle display off/on or open settings
      break;
    default:
      break;
  }
}
