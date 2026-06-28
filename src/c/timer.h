#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  bool     running;
  uint32_t elapsed_seconds;
  uint32_t last_fired_minute;   // last minute for which a vibration has been fired
} TimerState;

void timer_init(TimerState *t);
void timer_start(TimerState *t);
void timer_stop(TimerState *t);
void timer_reset(TimerState *t);

/* Advance one second. Returns true if this tick crossed a completed-minute
 * boundary where a vibration should fire (elapsed > 0, multiple of 60, and
 * not yet fired). Safe to call repeatedly.
 *
 * If the timer is not running, returns false and does not advance time. */
bool timer_tick(TimerState *t);

uint32_t timer_minutes(TimerState *t);
uint32_t timer_seconds_in_minute(TimerState *t);

/* Returns pixel width of the seconds progress bar as a fraction of
 * `display_width`. Pure arithmetic, no Pebble deps. */
uint32_t timer_seconds_bar_width(uint32_t elapsed_seconds, uint32_t display_width);
