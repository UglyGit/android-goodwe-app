# 12 - App State Integration Shell & Connection UI Recovery

Status: complete
Type: AFK
Blocked by: 07-dpad-routing.md, 11-polling-manager.md

## What to build
Manage central view switches and implement screen grey-outs via Q_PROPERTY properties when connections drop.

## Acceptance criteria

- First-run state shows setup; configured state shows dashboard.
- Connection loss exposes offline/stale state and disables live dashboard values.
- Reconnection restores live dashboard values.
- Settings action returns to setup view without stopping central state ownership.

## Verification

- Added `AppStateOrchestrator` public state interface and lifecycle test.
- `cmake -S . -B build && cmake --build build -j2` passed.
- `build/goodwe-test-runner -txt`: 12 passed, 0 failed.
