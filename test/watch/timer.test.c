#include <assert.h>
#include <stdio.h>

#include "../../src/c/timer.h"

static void test_init_starts_idle(void) {
  TimerState t;
  timer_init(&t);
  assert(!t.running);
  assert(t.elapsed_seconds == 0);
  assert(t.last_fired_minute == 0);
  assert(timer_minutes(&t) == 0);
  assert(timer_seconds_in_minute(&t) == 0);
}

static void test_start_runs_tick_advances(void) {
  TimerState t;
  timer_init(&t);
  timer_start(&t);

  for (int i = 0; i < 5; i++) {
    bool fired = timer_tick(&t);
    assert(!fired);
  }
  assert(t.elapsed_seconds == 5);
  assert(timer_minutes(&t) == 0);
  assert(timer_seconds_in_minute(&t) == 5);
}

static void test_tick_does_nothing_when_stopped(void) {
  TimerState t;
  timer_init(&t);
  /* not started yet */
  bool fired = timer_tick(&t);
  assert(!fired);
  assert(t.elapsed_seconds == 0);

  timer_start(&t);
  timer_stop(&t);
  fired = timer_tick(&t);
  assert(!fired);
  assert(t.elapsed_seconds == 0);
}

static void test_minute_boundary_fires_vibration(void) {
  TimerState t;
  timer_init(&t);
  timer_start(&t);

  /* Advance through minute 1 boundary */
  for (int i = 0; i < 59; i++) {
    bool fired = timer_tick(&t);
    assert(!fired);
  }
  /* tick 60 should fire */
  bool fired = timer_tick(&t);
  assert(fired);
  assert(t.elapsed_seconds == 60);
  assert(timer_minutes(&t) == 1);
  assert(t.last_fired_minute == 1);

  /* Subsequent ticks should not fire again until minute 2 */
  for (int i = 0; i < 59; i++) {
    assert(!timer_tick(&t));
  }
  fired = timer_tick(&t);
  assert(fired);
  assert(timer_minutes(&t) == 2);
  assert(t.last_fired_minute == 2);
}

static void test_reset_clears_last_fired_minute(void) {
  TimerState t;
  timer_init(&t);
  timer_start(&t);

  for (int i = 0; i < 60; i++) timer_tick(&t);
  assert(t.last_fired_minute == 1);

  timer_reset(&t);
  assert(t.last_fired_minute == 0);
  assert(t.elapsed_seconds == 0);
  assert(!t.running);
}

static void test_stop_preserves_last_fired_minute(void) {
  TimerState t;
  timer_init(&t);
  timer_start(&t);

  for (int i = 0; i < 60; i++) timer_tick(&t);
  assert(t.last_fired_minute == 1);

  timer_stop(&t);
  /* Stop does not reset last_fired_minute — resume should not
   * re-fire the same minute, and ticks while stopped are no-ops. */
  assert(!timer_tick(&t));
  assert(t.last_fired_minute == 1);

  /* Resume: advance 30 seconds, no boundary yet */
  timer_start(&t);
  for (int i = 0; i < 30; i++) {
    assert(!timer_tick(&t));
  }
  assert(t.elapsed_seconds == 90);
  assert(timer_minutes(&t) == 1);
  assert(t.last_fired_minute == 1);

  /* Advance 30 more seconds to minute-2 boundary — fires exactly once */
  bool fired = false;
  for (int i = 0; i < 30; i++) {
    if (timer_tick(&t)) {
      fired = true;
    } else {
      assert(!fired);
    }
  }
  assert(fired);
  assert(t.elapsed_seconds == 120);
  assert(timer_minutes(&t) == 2);
  assert(t.last_fired_minute == 2);
}

static void test_seconds_bar_width_linear(void) {
  const uint32_t W = 144;
  assert(timer_seconds_bar_width(0, W) == 0);
  assert(timer_seconds_bar_width(30, W) == 72);
  assert(timer_seconds_bar_width(60, W) == 0);   /* boundary reset */
  assert(timer_seconds_bar_width(90, W) == 72);
  /* At 59 seconds, floor(59 * 144 / 60) = 141 */
  assert(timer_seconds_bar_width(59, W) == 141);
}

int main(void) {
  test_init_starts_idle();
  test_start_runs_tick_advances();
  test_tick_does_nothing_when_stopped();
  test_minute_boundary_fires_vibration();
  test_reset_clears_last_fired_minute();
  test_stop_preserves_last_fired_minute();
  test_seconds_bar_width_linear();
  printf("timer.test: all %d tests passed\n", 7);
  return 0;
}
