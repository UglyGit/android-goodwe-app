# 10 - TelemetryMapper Two s Complement Vector Separation

Status: complete
Type: AFK
Blocked by: 09-payload-decoder.md

## What to build
Implement sign parsing logic routines to map absolute vectors and calculate load demand parameters.

## Acceptance criteria

- A device-facing public mapper entry point converts unsigned battery magnitude plus ETA mode `0x02`/`0x03` into discharging/charging direction.
- Solar, battery, and grid vectors expose absolute kW values with direction flags.
- House load calculates as solar generation + battery discharge - grid export, clamped at zero.

## Verification

- Added public-interface test covering charging/export and discharging/import device payloads.
- `cmake --build build -j2` passed.
- `build/goodwe-test-runner -txt`: 10 passed, 0 failed.
