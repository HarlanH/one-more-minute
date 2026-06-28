#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../src/c/vibration.h"

#define VIBE_SHORT_MS       125
#define VIBE_MEDIUM_MS      250
#define VIBE_LONG_MS        500
#define VIBE_SHORT_GAP_MS   100   /* within a group of identical symbols */
#define VIBE_LONG_GAP_MS    350   /* between different symbol groups (I/V/X) */

/* Helper: check the pattern matches expected durations exactly. */
static void expect_pattern(uint32_t minute,
                           const uint32_t *expected,
                           size_t expected_count) {
  uint32_t buf[MAX_VIBRATION_SEGMENTS] = {0};
  size_t n = vibration_pattern_for_minute(minute, buf, MAX_VIBRATION_SEGMENTS);
  if (n != expected_count) {
    printf("minute %u: expected %zu segments, got %zu. pattern: [", minute, expected_count, n);
    for (size_t i = 0; i < n && i < 16; i++) printf("%u, ", buf[i]);
    printf("]\n");
  }
  assert(n == expected_count);
  assert(0 == memcmp(buf, expected, expected_count * sizeof(uint32_t)));
}

/* I = single short pulse */
static void test_minute_1(void) {
  uint32_t expect[] = { VIBE_SHORT_MS };
  expect_pattern(1, expect, 1);
}

/* II = short, short_gap, short (same group -> short gap) */
static void test_minute_2_same_group_short_gap(void) {
  uint32_t expect[] = { VIBE_SHORT_MS, VIBE_SHORT_GAP_MS, VIBE_SHORT_MS };
  expect_pattern(2, expect, 3);
}

/* III = short, short_gap, short, short_gap, short */
static void test_minute_3(void) {
  uint32_t expect[] = {
    VIBE_SHORT_MS, VIBE_SHORT_GAP_MS,
    VIBE_SHORT_MS, VIBE_SHORT_GAP_MS,
    VIBE_SHORT_MS
  };
  expect_pattern(3, expect, 5);
}

/* IV = I then V: different groups -> long gap */
static void test_minute_4_subtractive_long_gap(void) {
  uint32_t expect[] = { VIBE_SHORT_MS, VIBE_LONG_GAP_MS, VIBE_MEDIUM_MS };
  expect_pattern(4, expect, 3);
}

/* V = single medium pulse */
static void test_minute_5(void) {
  uint32_t expect[] = { VIBE_MEDIUM_MS };
  expect_pattern(5, expect, 1);
}

/* VI = V then I: different groups -> long gap */
static void test_minute_6_long_gap(void) {
  uint32_t expect[] = { VIBE_MEDIUM_MS, VIBE_LONG_GAP_MS, VIBE_SHORT_MS };
  expect_pattern(6, expect, 3);
}

/* VIII = V, (long gap), I, (short gap), I, (short gap), I
 * This is the key spec from the user: long gap after V, short gaps between I's. */
static void test_minute_8_user_spec(void) {
  uint32_t expect[] = {
    VIBE_MEDIUM_MS, VIBE_LONG_GAP_MS,
    VIBE_SHORT_MS,  VIBE_SHORT_GAP_MS,
    VIBE_SHORT_MS,  VIBE_SHORT_GAP_MS,
    VIBE_SHORT_MS
  };
  expect_pattern(8, expect, 7);
}

/* IX = I then X: different groups -> long gap */
static void test_minute_9_subtractive_long_gap(void) {
  uint32_t expect[] = { VIBE_SHORT_MS, VIBE_LONG_GAP_MS, VIBE_LONG_MS };
  expect_pattern(9, expect, 3);
}

/* X = single long pulse */
static void test_minute_10(void) {
  uint32_t expect[] = { VIBE_LONG_MS };
  expect_pattern(10, expect, 1);
}

/* XIV = X, (long gap), I, (long gap), V
 * X to I is different (long), I to V within IV is different (long) */
static void test_minute_14(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS,   VIBE_LONG_GAP_MS,
    VIBE_SHORT_MS,  VIBE_LONG_GAP_MS,
    VIBE_MEDIUM_MS
  };
  expect_pattern(14, expect, 5);
}

/* XIX = X, (long gap), I, (long gap), X */
static void test_minute_19(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS,   VIBE_LONG_GAP_MS,
    VIBE_SHORT_MS,  VIBE_LONG_GAP_MS,
    VIBE_LONG_MS
  };
  expect_pattern(19, expect, 5);
}

/* XX = X, (short gap), X (same group -> short gap) */
static void test_minute_20_same_group(void) {
  uint32_t expect[] = { VIBE_LONG_MS, VIBE_SHORT_GAP_MS, VIBE_LONG_MS };
  expect_pattern(20, expect, 3);
}

/* XXIX = X, (short gap), X, (long gap), I, (long gap), X */
static void test_minute_29(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS,   VIBE_SHORT_GAP_MS,
    VIBE_LONG_MS,   VIBE_LONG_GAP_MS,
    VIBE_SHORT_MS,  VIBE_LONG_GAP_MS,
    VIBE_LONG_MS
  };
  expect_pattern(29, expect, 7);
}

/* XXXVIII = X,X,X (short gaps), (long gap), V, (long gap), I,I,I (short gaps) */
static void test_minute_38_max_symbols(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS,   VIBE_SHORT_GAP_MS,
    VIBE_LONG_MS,   VIBE_SHORT_GAP_MS,
    VIBE_LONG_MS,   VIBE_LONG_GAP_MS,
    VIBE_MEDIUM_MS, VIBE_LONG_GAP_MS,
    VIBE_SHORT_MS,  VIBE_SHORT_GAP_MS,
    VIBE_SHORT_MS,  VIBE_SHORT_GAP_MS,
    VIBE_SHORT_MS
  };
  expect_pattern(38, expect, 13);
}

/* XXXIX = X,X,X (short gaps), (long gap), I, (long gap), X */
static void test_minute_39_upper_bound(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS,   VIBE_SHORT_GAP_MS,
    VIBE_LONG_MS,   VIBE_SHORT_GAP_MS,
    VIBE_LONG_MS,   VIBE_LONG_GAP_MS,
    VIBE_SHORT_MS,  VIBE_LONG_GAP_MS,
    VIBE_LONG_MS
  };
  expect_pattern(39, expect, 9);
}

/* Minute 0 must return 0 segments */
static void test_minute_0_is_invalid(void) {
  uint32_t buf[MAX_VIBRATION_SEGMENTS] = {0};
  size_t n = vibration_pattern_for_minute(0, buf, MAX_VIBRATION_SEGMENTS);
  assert(n == 0);
}

/* Minute above ceiling returns 0 segments */
static void test_minute_over_ceiling_is_invalid(void) {
  uint32_t buf[MAX_VIBRATION_SEGMENTS] = {0};
  size_t n = vibration_pattern_for_minute(40, buf, MAX_VIBRATION_SEGMENTS);
  assert(n == 0);
}

int main(void) {
  test_minute_1();
  test_minute_2_same_group_short_gap();
  test_minute_3();
  test_minute_4_subtractive_long_gap();
  test_minute_5();
  test_minute_6_long_gap();
  test_minute_8_user_spec();
  test_minute_9_subtractive_long_gap();
  test_minute_10();
  test_minute_14();
  test_minute_19();
  test_minute_20_same_group();
  test_minute_29();
  test_minute_38_max_symbols();
  test_minute_39_upper_bound();
  test_minute_0_is_invalid();
  test_minute_over_ceiling_is_invalid();
  printf("vibration.test: all %d tests passed\n", 16);
  return 0;
}
