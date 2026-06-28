#include "timer.h"

#define SECONDS_PER_MINUTE 60U

void timer_init(TimerState *t) {
  t->running            = false;
  t->elapsed_seconds    = 0;
  t->last_fired_minute  = 0;
}

void timer_start(TimerState *t) {
  t->running = true;
}

void timer_stop(TimerState *t) {
  t->running = false;
}

void timer_reset(TimerState *t) {
  t->running            = false;
  t->elapsed_seconds    = 0;
  t->last_fired_minute  = 0;
}

bool timer_tick(TimerState *t) {
  if (!t->running) {
    return false;
  }

  t->elapsed_seconds += 1;

  bool at_boundary = (t->elapsed_seconds > 0) &&
                     (t->elapsed_seconds % SECONDS_PER_MINUTE == 0);

  uint32_t current_minute = timer_minutes(t);

  if (at_boundary && current_minute > t->last_fired_minute) {
    t->last_fired_minute = current_minute;
    return true;
  }
  return false;
}

uint32_t timer_minutes(TimerState *t) {
  return t->elapsed_seconds / SECONDS_PER_MINUTE;
}

uint32_t timer_seconds_in_minute(TimerState *t) {
  return t->elapsed_seconds % SECONDS_PER_MINUTE;
}

uint32_t timer_seconds_bar_width(uint32_t elapsed_seconds, uint32_t display_width) {
  uint32_t sec = elapsed_seconds % SECONDS_PER_MINUTE;
  return (uint32_t)((uint64_t)sec * display_width / SECONDS_PER_MINUTE);
}
