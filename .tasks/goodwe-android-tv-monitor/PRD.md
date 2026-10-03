# GoodWe Android TV Monitor

**Status:** Ready for implementation planning  
**Work ID:** goodwe-android-tv-monitor  
**Assumption:** New Android TV project built using Qt 6 / QML and C++; repository contains no existing app or ADRs.

## Problem Statement

An owner of a GoodWe GW9.999K-ETA-G20 inverter needs an always-visible, local Android TV dashboard for solar, grid, load, and battery state. Existing monitoring should not require cloud services, Home Assistant, a Raspberry Pi, or any inverter-control capability.

## Solution

Build a Qt/QML Android TV application. On first launch, the user enters the inverter IP address and port via a TV-remote friendly interface, tests the connection, and saves the configuration. The app background-polls the inverter through a custom C++ Modbus TCP client using read operations only, feeding live telemetry into a full-screen, declarative QML energy dashboard. Settings allow runtime connection changes.

## Reference Visuals

### Supplied Energy-Distribution Reference
*(Visual direction from source image /home/user/Downloads/132059076-36c35969-90b5-4f80-99dc-1633f9ada20d.png)*
* **Theme:** Deep dark background, radial Solar/Grid/Home/Battery layout with connector lines and directional flow markers.
* **Metrics:** Display live power in kW (not cumulative kWh).
* **Rings:** Battery node ring represents State of Charge (SOC) progress. Home node ring represents live calculated load power. Do not reproduce branding or icons.

### Dashboard Concept
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

### First-Run Configuration
```text
┌──────────────────────────────────────┐
│          GoodWe Monitor              │
│                                      │
│  Inverter IP address                 │
│       │
│                                      │
│  Port                                │
│       │
│                                      │
│       [ Test Connection ]            │
│                                      │
│       [ Save & Start ]               │
└──────────────────────────────────────┘
```

### Dashboard After Setup
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

### Local Network Topology
```text
                 Debian development PC (Running Qt Creator / CMake)
                       │
                       │ Wi-Fi / Ethernet Deployment (ADB)
                       ▼
              ┌─────────────────┐
              │ Android TV      │
              │ Qt/QML App      │
              └────────┬────────┘
                       │
                       │ LAN (Modbus TCP Client)
                       ▼
              ┌─────────────────┐
              │ GoodWe          │
              │ GW9.999K-ETA-G20│
              └─────────────────┘
```

### Read-Only Boundary
```text
GoodWe Monitor
     │
     ├── READ register  ✓
     ├── READ register  ✓
     ├── READ register  ✓
     └── WRITE register ✗
```

## User Stories

1. As an inverter owner, I want to enter the inverter IP address using a D-pad friendly text field, so the app can connect on my LAN.
2. As an inverter owner, I want to set or confirm port 502, so the app can connect to Modbus TCP.
3. As an inverter owner, I want to test the connection before saving, so bad settings are caught early.
4. As an inverter owner, I want invalid IP or port input rejected locally, so I can correct setup before connecting.
5. As an inverter owner, I want an unreachable IP or port reported cleanly, so I can fix network or inverter settings.
6. As an inverter owner, I want the app to switch instantly to the QML dashboard after setup, so the TV needs no routine interaction.
7. As an inverter owner, I want solar power displayed, so I can see generation.
8. As an inverter owner, I want grid power and import/export direction displayed, so I can understand grid flow.
9. As an inverter owner, I want load power displayed, so I can see household demand.
10. As an inverter owner, I want battery state of charge and charge/discharge power displayed, so I can understand battery use.
11. As an inverter owner, I want a 24-hour battery-SOC graph, so I can see daily battery behavior.
12. As an inverter owner, I want a clear stale/offline status display, so missing inverter data is not mistaken for live data.
13. As an inverter owner, I want unavailable dashboard values greyed out in QML when a connection fails, so none appear operative.
14. As an inverter owner, I want automatic background reconnection, so temporary LAN or inverter outages recover without remote usage.
15. As an inverter owner, I want the settings gear reachable via D-pad remote navigation, so I can change the IP or port at any time.
16. As a safety-conscious owner, I want the C++ core backend to contain no Modbus write mechanisms, ensuring the dashboard cannot control the inverter.

## Implementation Decisions

### Technical Stack & UI Boundary
* Core: C++20, Qt 6, CMake targeting Android TV 11 (API level 30, build RTMA.250416.2026, kernel 4.19.116++). Set minimum SDK to API 30 for this single-TV release.
* Frontend: Declarative QML utilizing QtQuick and QtQuick.Controls. All key navigation paths must explicitly handle D-pad focus loops (KeyNavigation). Large text scales must be used for a 10-foot TV viewing distance.
* Lifecyle: TV remains a normal television. The app runs as a standard foreground application. It does not start at boot, replace the system launcher, or spin up persistent Android OS services.

### Architectural Separation (For TDD Verification)
* InverterConfig (C++): Validates and stores host configuration using QSettings locally on the Android filesystem. Manual IP entry is required for MVP. Automatic LAN scan is deferred: its safe, reliable GoodWe identification method is not yet established.
* ModbusClient (C++): Built natively on top of QModbusTcpClient. This interface is fundamentally read-only; no write API pathways can be declared or implemented.
* TelemetryMapper (C++ / QObject): Direct conversion engine transforming raw Modbus registers into scaled values, mapping signs to directional metrics. Exposes data to QML via highly decoupled Q_PROPERTY bindings and notification signals.
* QML UI Views: Pure presentation layer bound directly to C++ properties. Dashboard layout follows supplied radial energy-distribution reference: Solar top, Grid left, Home right, Battery bottom. Keep a separate 24-hour SOC graph below or beside distribution layout, based on 16:9 available space. Battery SOC progress appears in the Battery node ring. Home node remains dedicated to live calculated load power.

### Data & Connection Rules
* Connection Contract: WiFi/LAN Kit 2.0 interface, TCP port 502, completely unauthenticated. Data contract still requires confirmed ETA-G20 register map, units, signed-power convention, and polling limits before live client work begins. Do not guess registers.
* Validation: Enforce IPv4 regex patterns and ports between 1 and 65535 locally prior to testing.
* Error Handling: Connection losses grey out elements in the QML tree. System failures must report the specific targeted IP and port as unreachable without speculating on hardware diagnostics. Do not claim why connection failed.
* Register Architecture: Utilizes explicit decimal Holding Registers. The C++ ModbusClient must account for zero-based API offsets before formatting requests. Read Holding Registers using Function Code 0x03.
* Atomic Block Reads: Use read-only FC03 block requests for each contiguous register group. The confirmed device map spans separate groups, so do not assume one block covers all dashboard fields.
* Diagnostics Boundary: PV voltage and current diagnostics are available at 606 and 608 for PV1, 614 and 616 for PV2. Grid Phase A voltage, current, and frequency are available at 622, 624, and 626. Do not show them in MVP dashboard.
* Polling Intervals: Poll live telemetry every 5 seconds while dashboard is visible. Stop polling when app is not visible.
* Calculated Values: House load is calculated, not read directly. Normalize every device-specific signed value first, then calculate: house load = PV generation + battery discharge power - grid export power. This handles both conventions where charging is positive and conventions where discharging is positive.
* Sign Conventions: Use supplied ETA mapping provisionally: positive battery power means charging and negative means discharging. The linked GoodWe ET community discussion uses opposite p_battery signs for a different model. Verify ETA direction during first live test and correct mapping if needed.
* History Persistence: Store SOC samples every 5 minutes in a rolling local JSON file so the 24-hour graph survives app restart. Retain only current rolling 24 hours. Plot missing history as visible gaps. Do not interpolate across app closure or unavailable inverter data.
* Architectural Boundaries: No cloud backend, external server, Home Assistant integration, or custom protocol framework.

# Data Contract & Modbus Map

All registers are read as Holding Registers using Modbus Function Code 0x03. Addresses use standard decimal notation.

| Dashboard Data        | Register (Dec) | Data Type      | Scale | Unit / Interpretation                                       |
| --------------------- | -------------- | -------------- | ----- | ----------------------------------------------------------- |
| **Serial Number**     | 35003 (8 regs) | STR (16 Bytes) | None  | ASCII inverter identification; confirmed live as `59999NBG266L1249` |
| **Solar Power (PV)**  | 35301 (2 regs) | UINT32         | 1     | W; confirmed live 0 W at night                             |
| **Grid Power Total**  | 35137 (2 regs) | INT32          | 1     | W; device-specific sign requires live import/export confirmation |
| **Load Power (Home)** | 35171 (2 regs) | INT32          | 1     | W; on-grid load, excludes backup load                       |
| **Battery Power**     | 35182 (2 regs) | UINT32         | 1     | W; direction comes from Battery Mode 35184                 |
| **Battery Mode**      | 35184          | UINT16         | 1     | `0x02` discharging, `0x03` charging                        |
| **Battery SOC**       | 37007          | UINT16         | 1     | %; confirmed live trend against GoodWe app                  |
| **Grid Frequency R/S/T** | 35123 / 35128 / 35133 | UINT16 | 0.01 | Hz; confirmed live as 50.00 Hz on all phases               |
| **Grid Voltage R/S/T** | 35121 / 35126 / 35131 | UINT16 | 0.1 | V                                                               |
| **Grid Current R/S/T** | 35122 / 35127 / 35132 | UINT16 | 0.1 | A                                                               |
| **Grid Power R/S/T** | 35124 / 35129 / 35134 (2 regs each) | INT32 | 1 | W; positive means inverter output/export                   |
| **Load Power R/S/T** | 35163 / 35165 / 35167 (2 regs each) | INT32 | 1 | W; on-grid per-phase load                                  |

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

### Live validation snapshot

Observed from inverter `192.168.1.57` during night read:

- Serial: `59999NBG266L1249`
- PV: `0 W`
- Grid frequency: `50.00 Hz` on R/S/T
- Grid total: `814 W` raw interpretation
- Load total: `794 W` raw interpretation
- Battery: `1013 W`, mode `0x02` (discharging)
- BMS SOC: `76%`

Power sign and word-order behavior remains provisional until daytime, charging, and discharging snapshots are captured.

## Out of Scope

- Any inverter control or Modbus write operation.
- Tariffs, billing, financial reporting, alerts, multi-inverter support, cloud sync, accounts, Home Assistant, Raspberry Pi, or separate server.
- Automatic LAN scan in first release.
- Register-map reverse engineering or guessing unsupported register values.
