# Release Notes — One More Minute

## Description

A count-up stopwatch app for the Pebble smartwatch. Run two independent stopwatches displayed in stacked zones, each with a seconds progress bar and play/pause indicator. Assign vibration feedback to either stopwatch and feel elapsed minutes as Roman numeral patterns (I, IV, V, IX, X…) directly on your wrist — perfect for cooking, focus sprints, or any task where glancing at a screen isn't enough.

---

## Version 1.1.0 — June 30, 2026

**Multi-platform support**

- Now supports all Pebble platforms except Aplite: basalt (Time/Time Steel), chalk (Time Round), diorite (Pebble 2), emery (Time 2), flint (Pebble 2 Duo), and gabbro (Round 2).
- Dynamic layout that fills each display size correctly.
- Rethought round-screen layout for Time Round (chalk) and Round 2 (gabbro) — content clusters near the center to avoid clipping at the circular edges.
- Minimal color: accent-colored progress bars and a color highlight on the vibe-assigned timer on color watches; unchanged black-and-white on diorite and flint.

---

## Version 1.0.0 — June 29, 2026

**Initial release**

- Two independent count-up stopwatches displayed in stacked zones on the Pebble 2 Duo.
- Each zone shows elapsed minutes (large center number), a 60-second progress bar, a play/pause/blank indicator, and a vibe assignment icon (outlined circle with arcs when active, bare circle when inactive).
- At most one stopwatch at a time can vibrate; assignment cycles through timer 1 → timer 2 → none via the SELECT button.
- Vibration patterns encode the current minute as a Roman numeral using short (125 ms), medium (250 ms), and long (500 ms) pulses with perceptually tuned gaps (100 ms within a symbol group, 350 ms between groups).
- Supports minutes 1–39; minute 0 is silent.
- Controls: UP/DOWN start-stop timers, long-press UP/DOWN clears a timer to 0:00, SELECT cycles vibe assignment, BACK exits the app.

---

## Version X.Y.Z — YYYY-MM-DD

*(placeholder — replace with upcoming release notes)*

- *Feature 1*
- *Feature 2*
- *Bug fixes and improvements*
