# 09 - Modbus Raw Payload Diagnostic & Decoding Engine

Status: complete
Type: AFK
Blocked by: 08-modbus-client.md

## What to build
Build binary byte-stretching functions parsing 16-bit registers into unified 32-bit fields.

## Verification

- Added public-interface tests for unsigned/signed 32-bit decoding, 16-bit decoding, and incomplete-buffer rejection.
- Fixed decoding to combine Qt Modbus register words directly without host/network byte swapping.
- `cmake --build build -j2` passed.
- `build/goodwe-test-runner -txt`: 9 passed, 0 failed.
