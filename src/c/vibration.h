#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * Convert a completed-minute count (1..MAX_VIBRATION_MINUTE) into a
 * sequence of vibe-duration segments (milliseconds). The sequence is
 * built so that the Roman numerals I, V, X map to kShort, kMedium,
 * kLong respectively; subtractive forms (IV = 4, IX = 9) are handled
 * explicitly. A silence gap is inserted between symbols.
 *
 * Minute 0 must NOT be passed here; caller is responsible for guarding.
 *
 * Returns the number of segments written into `durations` (0 on input
 * error). Caller must provide a buffer of at least MAX_VIBRATION_SEGMENTS.
 */

#define MAX_VIBRATION_MINUTE 39

/*  Worst case: XXXIX = XXX + IX
 *  = 3 longs + 6 inter-symbol gaps + 1 short + 1 inner gap + 1 long
 *  = 11 segments.  Rounded up for safety. */
#define MAX_VIBRATION_SEGMENTS 16

size_t vibration_pattern_for_minute(uint32_t minute,
                                    uint32_t *durations,
                                    size_t  capacity);
