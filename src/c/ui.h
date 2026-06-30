#pragma once

#include <pebble.h>

#include "timer.h"

/*
 * Vibrations can be assigned to timer 1, timer 2, or none.
 * Lives in main.c (single source of truth); ui.c reads it for rendering.
 */
typedef enum {
  VIBE_TIMER1 = 0,
  VIBE_TIMER2 = 1,
  VIBE_NONE   = 2,
} VibeAssignment;

typedef struct {
  TimerState *timers[2];
  VibeAssignment *vibe_assignment;   // writable back-pointer into main.c state
  Window *main_window;
  Layer  *root_layer;
  TextLayer *minute_text[2];
  char minute_buf[2][8];
  GBitmap *bmp_play;
  GBitmap *bmp_pause;
  GBitmap *bmp_vibe_on;
  GBitmap *bmp_vibe_off;
  int16_t display_width;
  int16_t display_height;
  int16_t zone_height;
} UIContext;

/* Build the UI. Pass pointers to the two TimerStates and the shared
 * VibeAssignment. The UI renders from them on demand (never owns them). */
void ui_init(UIContext *ctx,
             TimerState *timer1, TimerState *timer2,
             VibeAssignment *vibe_assignment);

/* Push the window onto the stack and begin showing it. */
void ui_push(UIContext *ctx);

/* Tear down all layers. */
void ui_destroy(UIContext *ctx);

/* Re-render the root layer and update the minutes text layers.
 * Caller should invoke after every state change (tick / click / vibe cycle). */
void ui_refresh(UIContext *ctx);
