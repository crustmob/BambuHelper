#ifndef TOUCH_INPUT_H
#define TOUCH_INPUT_H

#include <stdint.h>

typedef enum {
    GESTURE_NONE = 0,
    GESTURE_TAP,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN
} TouchGesture;

void initTouch();
bool readTouch(int32_t *x, int32_t *y);
TouchGesture readGesture();
void handleTouch();
void handleTouchGesture(TouchGesture gesture);

#endif // TOUCH_INPUT_H
