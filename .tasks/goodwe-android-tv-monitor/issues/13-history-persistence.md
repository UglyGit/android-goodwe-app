# 13 - 5-Minute Sample Rolling Ledger Persistence Engine

Status: complete
Type: AFK
Blocked by: 10-telemetry-mapper.md

## What to build
Log battery metrics to rolling JSON arrays every 300 seconds, capturing explicit timestamp gaps.

## Acceptance criteria

- Public history repository stores timestamped SOC samples in local JSON.
- Samples older than 24 hours are removed when loading or adding samples.
- Missing samples remain gaps; no interpolation is performed.
- Persisted samples reload after repository recreation.

## Verification

- `cmake -S . -B build && cmake --build build -j2` passed.
- `build/goodwe-test-runner -txt`: 13 passed, 0 failed.
