#include "vibration.h"
#include <stdbool.h>

#define VIBE_SHORT_MS       125
#define VIBE_MEDIUM_MS      250
#define VIBE_LONG_MS        500
#define VIBE_SHORT_GAP_MS   100
#define VIBE_LONG_GAP_MS    350

typedef enum { SYM_NONE = 0, SYM_I, SYM_V, SYM_X } Sym;

static inline size_t append_val(uint32_t *buf, size_t cap, size_t n, uint32_t v) {
  if (n < cap) buf[n] = v;
  return n + 1;
}

static size_t emit_pulse(uint32_t *buf, size_t cap, size_t n,
                         Sym sym, Sym *last) {
  if (*last != SYM_NONE) {
    uint32_t gap = (*last == sym) ? VIBE_SHORT_GAP_MS : VIBE_LONG_GAP_MS;
    n = append_val(buf, cap, n, gap);
  }
  uint32_t dur = (sym == SYM_I) ? VIBE_SHORT_MS
              : (sym == SYM_V) ? VIBE_MEDIUM_MS
              :                  VIBE_LONG_MS;
  n = append_val(buf, cap, n, dur);
  *last = sym;
  return n;
}

size_t vibration_pattern_for_minute(uint32_t minute,
                                    uint32_t *durations,
                                    size_t  capacity) {
  if (minute == 0 || minute > MAX_VIBRATION_MINUTE) {
    return 0;
  }

  size_t n = 0;
  Sym last = SYM_NONE;

  uint32_t tens = minute / 10;
  uint32_t ones = minute % 10;

  for (uint32_t i = 0; i < tens; i++) {
    n = emit_pulse(durations, capacity, n, SYM_X, &last);
  }

  if (ones == 4) {
    n = emit_pulse(durations, capacity, n, SYM_I, &last);
    n = emit_pulse(durations, capacity, n, SYM_V, &last);
  } else if (ones == 9) {
    n = emit_pulse(durations, capacity, n, SYM_I, &last);
    n = emit_pulse(durations, capacity, n, SYM_X, &last);
  } else {
    if (ones >= 5) {
      n = emit_pulse(durations, capacity, n, SYM_V, &last);
      ones -= 5;
    }
    for (uint32_t i = 0; i < ones; i++) {
      n = emit_pulse(durations, capacity, n, SYM_I, &last);
    }
  }

  return n;
}
