# 11 - Background Polling Lifecycle Manager

Status: complete
Type: AFK
Blocked by: 10-telemetry-mapper.md

## What to build
Coordinate async background 5-second interval execution loop triggers and connection heartbeats.

## Acceptance criteria

- Polling interval is 5 seconds.
- Starting while visible triggers an immediate read and schedules recurring reads.
- Hiding stops timer activity; becoming visible resumes if polling was requested.
- Explicit stop prevents further polling.

## Verification

- Added `PollingManager` public lifecycle test.
- `cmake --build build -j2` passed.
- `build/goodwe-test-runner -txt`: 11 passed, 0 failed.
