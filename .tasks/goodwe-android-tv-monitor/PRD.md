# GoodWe Android TV Monitor

**Status:** Ready for implementation planning  
**Work ID:** `goodwe-android-tv-monitor`  
**Assumption:** New Android project; repository contains no existing app or ADRs.

## Problem Statement

An owner of a GoodWe GW9.999K-ETA-G20 inverter needs an always-visible, local Android TV dashboard for solar, grid, load, and battery state. Existing monitoring should not require cloud services, Home Assistant, a Raspberry Pi, or any inverter-control capability.

## Solution

Build a Kotlin Android TV app using Jetpack Compose. On first launch, user enters inverter IP address and port, tests connection, then saves configuration. App polls inverter through Modbus TCP using read operations only and shows a full-screen, TV-remote-friendly energy dashboard. Settings allow connection changes.

## Reference visuals

### Supplied energy-distribution reference

![Dark radial energy-distribution dashboard: Solar at top, Grid at left, Home at right, Battery at bottom](/home/user/Downloads/132059076-36c35969-90b5-4f80-99dc-1633f9ada20d.png)

Use this as visual direction: dark background, radial Solar/Grid/Home/Battery layout, coloured source rings, connector lines, and directional flow markers. Display live power in kW rather than the reference image's accumulated kWh figures. Battery ring is SOC progress. Home ring is live load power, not SOC. Do not reproduce its icons or branding.

### Dashboard concept

```text
┌──────────────────────────────────────────────────────────┐
│                                                          │
│       ☀ SOLAR          GRID              LOAD            │
│       3.25 kW          ↓ 0.82 kW         2.43 kW         │
│                                                          │
│                         ↓                                │
│                    ┌─────────┐                           │
│       ────────────▶│ BATTERY │                           │
│                    │   82%   │                           │
│                    │  -1.64kW│                           │
│                    └─────────┘                           │
│                                                          │
│  Battery State of Charge                                 │
│  100% ┤                 ╭──────╮                         │
│   80% ┤──────╮      ╭───╯      ╰──╮                     │
│   60% ┤      ╰──────╯              ╰────                │
│       └──────────────────────────────────────────        │
│        00:00       06:00       12:00       18:00         │
└──────────────────────────────────────────────────────────┘
```

### First-run configuration

```text
┌──────────────────────────────────────┐
│          GoodWe Monitor              │
│                                      │
│  Inverter IP address                 │
│  [ 192.168.1.123              ]      │
│                                      │
│  Port                                │
│  [ 502                        ]      │
│                                      │
│       [ Test Connection ]            │
│                                      │
│       [ Save & Start ]               │
└──────────────────────────────────────┘
```

### Dashboard after setup

```text
                 ☀ SOLAR
                  3.25 kW
                     │
                     ▼
       ⚡ GRID ───────┼────── 🏠 LOAD
        0.82 kW       │        2.43 kW
                     ▲
                     │
                 🔋 BATTERY
                    82%
                  -1.64 kW

       ─────────────────────────────

       Battery SOC
       100% ┤
        80% ┤────────╮
        60% ┤        ╰────────╮
        40% ┤                 ╰──
            └─────────────────────
             00   04   08   12   16   20   24
```

### Local network topology

```text
                 Debian development PC
                       │
                       │ Wi-Fi/Ethernet
                       ▼
              ┌─────────────────┐
              │ Android TV      │
              │ Dashboard App   │
              └────────┬────────┘
                       │
                       │ LAN
                       ▼
              ┌─────────────────┐
              │ GoodWe          │
              │ GW9.999K-ETA-G20│
              │ Modbus TCP      │
              └─────────────────┘
```

### Read-only boundary

```text
GoodWe Monitor
     │
     ├── READ register  ✓
     ├── READ register  ✓
     ├── READ register  ✓
     └── WRITE register ✗
```

## User Stories

1. As an inverter owner, I want to enter inverter IP address, so app can connect on my LAN.
2. As an inverter owner, I want to set or confirm port 502, so app can connect to Modbus TCP.
3. As an inverter owner, I want to test connection before saving, so bad settings are caught early.
4. As an inverter owner, I want invalid IP or port input rejected, so I can correct setup before connecting.
5. As an inverter owner, I want an unreachable IP or port reported, so I can fix network or inverter settings.
6. As an inverter owner, I want app to start on dashboard after setup, so TV needs no routine interaction.
7. As an inverter owner, I want solar power displayed, so I can see generation.
8. As an inverter owner, I want grid power and import/export direction displayed, so I can understand grid flow.
9. As an inverter owner, I want load power displayed, so I can see household demand.
10. As an inverter owner, I want battery state of charge and charge/discharge power displayed, so I can understand battery use.
11. As an inverter owner, I want a 24-hour battery-SOC graph, so I can see daily battery behavior.
12. As an inverter owner, I want clear stale/offline status, so missing inverter data is not mistaken for live data.
13. As an inverter owner, I want unavailable dashboard values greyed out when connection fails, so none appear operative.
14. As an inverter owner, I want automatic reconnect, so temporary LAN or inverter outages recover without remote use.
15. As an inverter owner, I want settings reachable with TV remote, so I can change IP or port.
16. As a safety-conscious owner, I want app to issue no Modbus writes, so dashboard cannot control inverter.

## Implementation Decisions

- Kotlin, Android TV, Jetpack Compose. Target D-pad navigation, readable large text, full-screen dashboard.
- Dashboard layout follows supplied radial energy-distribution reference: Solar top, Grid left, Home right, Battery bottom. Keep a separate 24-hour SOC graph below or beside distribution layout, based on 16:9 available space.
- Battery SOC progress appears in the Battery node ring. Home node remains dedicated to live calculated load power.
- Deployment target is Android TV 11, API level 30, build `RTMA.250416.2026`, kernel `4.19.116++`. Set minimum SDK to API 30 for this single-TV release.
- TV remains a normal television. App does not start at boot, replace the launcher, or run as a persistent foreground service.
- Separate configuration, Modbus read client, telemetry mapping, polling/reconnect, and dashboard state. Modbus client exposes read operations only; no write API exists.
- Store IP address and port in app settings. Default port is 502. No credentials are collected or stored.
- First-run state: configuration form. Saved valid configuration: dashboard. Failed or missing connection: grey every dashboard element and show no operative values. Settings cog remains active for repair.
- Validate IPv4 or hostname input locally and require port 1 through 65535 before test/save. A failed TCP connection reports the entered IP and port as unreachable. Do not claim why connection failed.
- Connection contract: WiFi/LAN Kit 2.0 IP address, TCP port 502, no authentication. Data contract still requires confirmed ETA-G20 register map, units, signed-power convention, and polling limits before live client work begins. Do not guess registers.
- Read Holding Registers. Register addresses below use documented decimal addresses. Verify any client-library zero-based offset before sending a request.

  | Dashboard data | Register | Type | Scale | Rule |
  | --- | ---: | --- | --- | --- |
  | Serial number | 512 | STR, 16 bytes | None | Identification |
  | Model name | 528 | STR, 10 bytes | None | Identification |
  | Work mode | 544 | UINT16 | None | 0 wait, 1 normal, 2 discharge/off-grid, 3 fault |
  | PV1 power | 610 | UINT32 | 1 W | Solar input |
  | PV2 power | 618 | UINT32 | 1 W | Solar input |
  | Grid power | 632 | INT32 | 1 W | Positive export. Negative import. |
  | Battery power | 648 | INT32 | 1 W | Provisional: positive charge, negative discharge. Verify on first live test. |
  | Battery SOC | 650 | UINT16 | 1% | State of charge |
  | Battery SOH | 651 | UINT16 | 1% | State of health |
  | Battery temperature | 652 | INT16 | 0.1 C | Battery temperature |
  | Total PV generation | 674 | UINT32 | 0.1 kWh | Accumulator |
  | Grid export total | 678 | UINT32 | 0.1 kWh | Accumulator |
  | Grid import total | 682 | UINT32 | 0.1 kWh | Accumulator |
  | House-load total | 686 | UINT32 | 0.1 kWh | Accumulator |

- PV voltage and current diagnostics are available at 606 and 608 for PV1, 614 and 616 for PV2. Grid Phase A voltage, current, and frequency are available at 622, 624, and 626. Do not show them in MVP dashboard.
- House load is calculated, not read directly. Normalize every device-specific signed value first, then calculate: `house load = PV generation + battery discharge power - grid export power`. This handles both conventions where charging is positive and conventions where discharging is positive.
- Use supplied ETA mapping provisionally: positive battery power means charging and negative means discharging. The linked GoodWe ET community discussion uses opposite `p_battery` signs for a different model. Verify ETA direction during first live test and correct mapping if needed.
- Persist SOC samples every 5 minutes so 24-hour graph survives app restart. Retain only current rolling 24 hours.
- Plot missing history as visible gaps. Do not interpolate across app closure or unavailable inverter data.
- Poll live telemetry every 5 seconds while dashboard is visible. Stop polling when app is not visible.
- Manual IP entry is required MVP. Automatic LAN scan is deferred: its safe, reliable GoodWe identification method is not yet established.
- No cloud backend, external server, Home Assistant integration, or custom protocol framework.

## Testing Decisions

- Tests verify observable behavior through public interfaces, never private implementation.
- Test telemetry mapping with documented register data: UINT16, INT16, UINT32, INT32, string decoding, scale factors, signed grid/battery direction, invalid values, and unavailable registers.
- Test live-direction calibration during first device test: compare raw battery value with a known charging interval and a known discharging interval. Correct sign mapping if device contradicts provisional convention.
- Test computed load from normalized power directions. Verify charging, discharging, grid import, and grid export cases.
- Test connection state: first run requires setup, valid saved configuration opens dashboard, failed polling becomes offline/stale, reconnect restores live state.
- Test offline presentation: no live value or graph is shown as operative, all dashboard content is greyed out, and Settings cog remains D-pad accessible.
- Test SOC history gaps after missing samples. Graph must not draw a continuous line across them.
- Test input validation and connection errors: reject malformed IP/hostname and ports outside 1 through 65535; report a failed TCP connection as unreachable.
- Test read-only contract using a fake transport that records requests: reads permitted; no write request can be created or sent.
- Test IP and port persistence through app-facing configuration repository.
- Test 24-hour history retention and dashboard-ready graph samples.
- Add Android UI tests for D-pad focus and Settings access only where emulator/device setup makes them stable.

## Out of Scope

- Any inverter control or Modbus write operation.
- Tariffs, billing, financial reporting, alerts, multi-inverter support, cloud sync, accounts, Home Assistant, Raspberry Pi, or separate server.
- Automatic LAN scan in first release.
- Register-map reverse engineering or guessing unsupported register values.

## Further Notes

- Physical target: GoodWe GW9.999K-ETA-G20 on local network, Android/Google TV on same LAN.
- Target model confirmed: `GW9.999K-ETA-G20`. Use ETA mapping only.
- Confirm Android TV can reach inverter directly before UI implementation.
- Next planning action: split PRD into implementation issues after Modbus TCP details are available; a setup/dashboard shell can proceed independently.
- Source context included ASCII drawings only. No bitmap images were supplied.
