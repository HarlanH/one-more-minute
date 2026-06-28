#include <pebble.h>

#include "timer.h"
#include "vibration.h"
#include "ui.h"

#define TICK_INTERVAL_MS 1000

static TimerState s_timers[2];
static VibeAssignment s_vibe_assignment = VIBE_NONE;
static UIContext s_ui;
static AppTimer *s_tick_timer = NULL;

static void tick_handler(void *data);
static void schedule_tick(void);

static void fire_vibration_for_minute(uint32_t minute) {
  static uint32_t pattern[MAX_VIBRATION_SEGMENTS];
  size_t n = vibration_pattern_for_minute(minute, pattern, MAX_VIBRATION_SEGMENTS);
  if (n == 0) return;
  VibePattern pebble_pattern;
  pebble_pattern.durations = pattern;
  pebble_pattern.num_segments = (uint32_t)n;
  vibes_enqueue_custom_pattern(pebble_pattern);
}

static bool any_timer_running(void) {
  return s_timers[0].running || s_timers[1].running;
}

static void schedule_tick(void) {
  s_tick_timer = app_timer_register(TICK_INTERVAL_MS, tick_handler, NULL);
}

static void tick_handler(void *data) {
  (void)data;
  s_tick_timer = NULL;

  for (int i = 0; i < 2; i++) {
    bool crossed = timer_tick(&s_timers[i]);
    if (crossed) {
      VibeAssignment mine = (i == 0) ? VIBE_TIMER1 : VIBE_TIMER2;
      if (mine == s_vibe_assignment) {
        fire_vibration_for_minute(timer_minutes(&s_timers[i]));
      }
    }
  }

  ui_refresh(&s_ui);

  if (any_timer_running()) schedule_tick();
}

static void maybe_start_tick_loop(void) {
  if (s_tick_timer == NULL && any_timer_running()) schedule_tick();
}

static void up_click_handler(ClickRecognizerRef ref, void *context) {
  (void)ref;
  (void)context;
  if (s_timers[0].running) {
    timer_stop(&s_timers[0]);
  } else {
    timer_start(&s_timers[0]);
    maybe_start_tick_loop();
  }
  ui_refresh(&s_ui);
}

static void up_long_click_handler(ClickRecognizerRef ref, void *context) {
  (void)ref;
  (void)context;
  timer_reset(&s_timers[0]);
  if (!any_timer_running()) { /* tick loop will wind down on next fire */ }
  ui_refresh(&s_ui);
}

static void down_click_handler(ClickRecognizerRef ref, void *context) {
  (void)ref;
  (void)context;
  if (s_timers[1].running) {
    timer_stop(&s_timers[1]);
  } else {
    timer_start(&s_timers[1]);
    maybe_start_tick_loop();
  }
  ui_refresh(&s_ui);
}

static void down_long_click_handler(ClickRecognizerRef ref, void *context) {
  (void)ref;
  (void)context;
  timer_reset(&s_timers[1]);
  if (!any_timer_running()) { /* tick loop will wind down */ }
  ui_refresh(&s_ui);
}

static void select_click_handler(ClickRecognizerRef ref, void *context) {
  (void)ref;
  (void)context;
  s_vibe_assignment = (VibeAssignment)((s_vibe_assignment + 1) % 3);
  ui_refresh(&s_ui);
}

static void click_config_provider(void *context) {
  (void)context;
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  /* BACK pops the main window and exits the app */
}

static void init(void) {
  timer_init(&s_timers[0]);
  timer_init(&s_timers[1]);

  ui_init(&s_ui, &s_timers[0], &s_timers[1], &s_vibe_assignment);
  ui_refresh(&s_ui);

  window_set_click_config_provider_with_context(s_ui.main_window,
                                                click_config_provider, NULL);
  ui_push(&s_ui);
}

static void deinit(void) {
  ui_destroy(&s_ui);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
