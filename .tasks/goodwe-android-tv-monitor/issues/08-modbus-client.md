# 08 - Read-Only ModbusClient Atomic Block Reader

Status: complete
Type: AFK
Blocked by: 03-test-framework.md

## What to build
Write an async QModbusTcpClient module restricted exclusively to read requests.

## Acceptance criteria

- A connected client issues asynchronous, read-only Function Code `0x03` requests for confirmed contiguous register groups.
- The confirmed dashboard map uses device addresses directly: serial `35003`, phase/grid/load/battery groups in `35121` through `35184`, PV total `35301`, and SOC `37007`.
- Failed or unavailable requests emit `readFailed()` and never create a write request.

## Verification

- Added public-interface test for rejected reads while disconnected.
- `cmake --build build -j2` passed.
- `build/goodwe-test-runner -txt`: 9 passed, 0 failed.
- Simulator client completed a live block read against `127.0.0.1:5020`.
- Live inverter read from `192.168.1.57:502` confirmed serial `59999NBG266L1249` and 50.00 Hz on R/S/T.
