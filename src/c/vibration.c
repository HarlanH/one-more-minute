#include "vibration.h"
#include <stdbool.h>

#define VIBE_SHORT_MS   125
#define VIBE_MEDIUM_MS  250
#define VIBE_LONG_MS    500
#define VIBE_GAP_MS     75

static inline size_t append_val(uint32_t *buf, size_t cap, size_t n, uint32_t v) {
  if (n < cap) buf[n] = v;
  return n + 1;
}

static inline size_t maybe_gap(uint32_t *buf, size_t cap, size_t n) {
  if (n > 0) {
    return append_val(buf, cap, n, VIBE_GAP_MS);
  }
  return n;
}

static size_t build_tens(uint32_t tens, uint32_t *buf, size_t cap, size_t n) {
  for (uint32_t i = 0; i < tens; i++) {
    n = maybe_gap(buf, cap, n);
    n = append_val(buf, cap, n, VIBE_LONG_MS);
  }
  return n;
}

static size_t build_ones(uint32_t rem, uint32_t *buf, size_t cap, size_t n) {
  if (rem == 0) return n;

  if (rem == 4) {
    n = maybe_gap(buf, cap, n);
    n = append_val(buf, cap, n, VIBE_SHORT_MS);
    n = maybe_gap(buf, cap, n);
    n = append_val(buf, cap, n, VIBE_MEDIUM_MS);
    return n;
  }
  if (rem == 5) {
    n = maybe_gap(buf, cap, n);
    n = append_val(buf, cap, n, VIBE_MEDIUM_MS);
    return n;
  }
  if (rem == 9) {
    n = maybe_gap(buf, cap, n);
    n = append_val(buf, cap, n, VIBE_SHORT_MS);
    n = maybe_gap(buf, cap, n);
    n = append_val(buf, cap, n, VIBE_LONG_MS);
    return n;
  }

  bool needs_v = (rem >= 6 && rem <= 8);
  uint32_t num_i = (rem >= 5) ? (rem - 5) : rem;

  if (needs_v) {
    n = maybe_gap(buf, cap, n);
    n = append_val(buf, cap, n, VIBE_MEDIUM_MS);
  }
  for (uint32_t i = 0; i < num_i; i++) {
    n = maybe_gap(buf, cap, n);
    n = append_val(buf, cap, n, VIBE_SHORT_MS);
  }
  return n;
}

size_t vibration_pattern_for_minute(uint32_t minute,
                                    uint32_t *durations,
                                    size_t  capacity) {
  if (minute == 0 || minute > MAX_VIBRATION_MINUTE) {
    return 0;
  }

  size_t n = build_tens(minute / 10, durations, capacity, 0);
  n = build_ones(minute % 10, durations, capacity, n);
  return n;
}
