# Agent Instructions

## Project Type

Pebble watch app (C + PebbleKit JS). Build toolchain: Waf + `pebble` CLI. See `../anylist-pebble/` for the closest working example.

## Frequently Used Commands

```bash
# Build
pebble build

# Install on physical watch (Flint / Pebble 2 Duo)
pebble install --phone <PHONE_IP>

# Install on emulator
pebble install --emulator flint

# Watch device logs
pebble logs --phone <PHONE_IP>

# JS tests (Node built-in test runner)
npm run test:js

# C unit tests (compiled with cc, run from npm)
npm run test:watch
```

## Source Layout

```
src/c/          # Watch app — C source
src/pkjs/       # PebbleKit JS — phone-side runtime
test/pkjs/      # JS tests (*.test.js)
test/watch/     # C tests (*.test.c) — compiled directly, no test framework
resources/      # Images (icons, PNGs) listed in package.json
scripts/        # Build helpers, investigation scripts
plans/          # Design docs, feature plans
wscript         # Waf build config (loads pebble_sdk)
package.json    # App manifest (UUID, messageKeys, resources)
```

## Pebble Build Gotchas

- `wscript` compiles only `src/c/**/*.c`. JS in `src/pkjs/` is bundled by `pbl_bundle` (excluding `index.js`). Do not add .c files outside `src/c/` without updating the wscript.
- `messageKeys` in `package.json` are integer-keyed AppMessage slots. Adding a new key requires a free slot number — check existing assignments before adding.
- Target platform `flint` = Pebble 2 Duo (C SDK only). `emery` / `gabbro` can use Alloy. Changing platforms requires SDK adjustments.

## PebbleKit JS Constraints

- PKJS runs in a JS-Core sandbox on the phone. It supports `fetch()`, `localStorage`, and XHR. It does **not** support Node APIs: no `require`, `fs`, `got`, `Buffer`, or `process`.
- Protobuf / binary uploads must use `Uint8Array`, never JS strings.

## Credentials and Security

- Never commit `.env`, `credentials.env`, `config.local.js`, `credentials.json`, or any file containing secrets.
- Use `config.local.example.js` as a template. The build script (`scripts/build.sh`) injects credentials into the JS bundle at build time and restores the source afterwards.
- All credential file patterns are already in `.gitignore`.

## Testing Notes

- JS tests use `node --test` (Node built-in runner, zero dependencies). Test files live in `test/pkjs/**/*.test.js`.
- C tests are plain C files compiled with `cc -std=c99 -Wall -Wextra`. No test framework — assertions via `assert.h` or custom macros. The `test:watch` script compiles and runs each test binary sequentially.
- When adding a C test, wire it into the `test:watch` npm script.

## AppMessage Conventions

- Inbox size should be `4096` when sending multi-string payloads.
- Chunk large list payloads (10 items/chunk is the established pattern).
- Include a generation counter (e.g. `LIST_GENERATION`) so the watch can reject stale/interleaved streams.
- The watch builds a flat display-row array (headers + items). Toggles must map display-index back to source-index.

## Workflow

- Prefer reusable scripts in `scripts/` for protocol or API investigation before making app-code changes.
- For device debugging, capture logs in one persistent session (`pebble logs`) instead of repeated start/kill loops.
- Plans go in `plans/`. Architecture decisions go in a top-level `ARCHITECTURE.md`. Work tracking in `TODO.md`.
