#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../src/c/vibration.h"

#define VIBE_SHORT_MS   125
#define VIBE_MEDIUM_MS  250
#define VIBE_LONG_MS    500
#define VIBE_GAP_MS     75

/* Helper: check the pattern matches expected durations exactly. */
static void expect_pattern(uint32_t minute,
                           const uint32_t *expected,
                           size_t expected_count) {
  uint32_t buf[MAX_VIBRATION_SEGMENTS] = {0};
  size_t n = vibration_pattern_for_minute(minute, buf, MAX_VIBRATION_SEGMENTS);
  if (n != expected_count) {
    printf("minute %u: expected %zu segments, got %zu. pattern: [", minute, expected_count, n);
    for (size_t i = 0; i < n && i < 10; i++) printf("%u, ", buf[i]);
    printf("]\n");
  }
  assert(n == expected_count);
  assert(0 == memcmp(buf, expected, expected_count * sizeof(uint32_t)));
}

/* I = 1 short pulse */
static void test_minute_1_is_short(void) {
  uint32_t expect[] = { VIBE_SHORT_MS };
  expect_pattern(1, expect, 1);
}

/* II = short, gap, short */
static void test_minute_2(void) {
  uint32_t expect[] = { VIBE_SHORT_MS, VIBE_GAP_MS, VIBE_SHORT_MS };
  expect_pattern(2, expect, 3);
}

/* IV (4) = short, gap, medium */
static void test_minute_4_subtractive(void) {
  uint32_t expect[] = { VIBE_SHORT_MS, VIBE_GAP_MS, VIBE_MEDIUM_MS };
  expect_pattern(4, expect, 3);
}

/* V = medium */
static void test_minute_5(void) {
  uint32_t expect[] = { VIBE_MEDIUM_MS };
  expect_pattern(5, expect, 1);
}

/* VI = medium, gap, short */
static void test_minute_6(void) {
  uint32_t expect[] = { VIBE_MEDIUM_MS, VIBE_GAP_MS, VIBE_SHORT_MS };
  expect_pattern(6, expect, 3);
}

/* IX (9) = short, gap, long */
static void test_minute_9_subtractive(void) {
  uint32_t expect[] = { VIBE_SHORT_MS, VIBE_GAP_MS, VIBE_LONG_MS };
  expect_pattern(9, expect, 3);
}

/* X = long */
static void test_minute_10(void) {
  uint32_t expect[] = { VIBE_LONG_MS };
  expect_pattern(10, expect, 1);
}

/* XIV = long, gap, short, gap, medium */
static void test_minute_14(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_SHORT_MS, VIBE_GAP_MS,
    VIBE_MEDIUM_MS
  };
  expect_pattern(14, expect, 5);
}

/* XIX = long, gap, short, gap, long */
static void test_minute_19(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_SHORT_MS, VIBE_GAP_MS,
    VIBE_LONG_MS
  };
  expect_pattern(19, expect, 5);
}

/* XX = long, gap, long */
static void test_minute_20(void) {
  uint32_t expect[] = { VIBE_LONG_MS, VIBE_GAP_MS, VIBE_LONG_MS };
  expect_pattern(20, expect, 3);
}

/* XXIX = long, gap, long, gap, short, gap, long */
static void test_minute_29(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_SHORT_MS, VIBE_GAP_MS,
    VIBE_LONG_MS
  };
  expect_pattern(29, expect, 7);
}

/* XXXIX = XXX + IX = long, gap, long, gap, long, gap, short, gap, long */
static void test_minute_39_upper_bound(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_SHORT_MS, VIBE_GAP_MS,
    VIBE_LONG_MS
  };
  expect_pattern(39, expect, 9);
}

/* Minute 0 must return 0 segments (caller guards this, but API should) */
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

/* XXXVIII = 3 longs + medium + 3 shorts = 3,3,1,1,3 short,3 short = ?
 * Actually XXXVIII = 30 + 5 + 3 = X,X,X,V,I,I,I
 * = long, gap, long, gap, long, gap, medium, gap, short, gap, short, gap, short
 * = 13 segments */
static void test_minute_38(void) {
  uint32_t expect[] = {
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_LONG_MS, VIBE_GAP_MS,
    VIBE_MEDIUM_MS, VIBE_GAP_MS,
    VIBE_SHORT_MS, VIBE_GAP_MS,
    VIBE_SHORT_MS, VIBE_GAP_MS,
    VIBE_SHORT_MS
  };
  expect_pattern(38, expect, 13);
}

int main(void) {
  test_minute_1_is_short();
  test_minute_2();
  test_minute_4_subtractive();
  test_minute_5();
  test_minute_6();
  test_minute_9_subtractive();
  test_minute_10();
  test_minute_14();
  test_minute_19();
  test_minute_20();
  test_minute_29();
  test_minute_39_upper_bound();
  test_minute_0_is_invalid();
  test_minute_over_ceiling_is_invalid();
  test_minute_38();
  printf("vibration.test: all %d tests passed\n", 15);
  return 0;
}
