#include "telemetry_mapper.h"
#include <cmath>

TelemetryMapper::TelemetryMapper(QObject *parent)
    : QObject(parent)
{
}

void TelemetryMapper::processRawTelemetry(uint32_t rawPv, int32_t rawBattery, int32_t rawGrid, uint16_t rawSoc)
{
    updateTelemetry(rawPv, rawBattery, rawGrid, rawSoc);
}

void TelemetryMapper::processRawTelemetry(uint32_t rawPv, uint32_t rawBattery, uint16_t batteryMode,
                                          int32_t rawGrid, uint16_t rawSoc)
{
    // GoodWe ETA mode: 0x02 = discharging, 0x03 = charging.
    const int32_t signedBattery = batteryMode == 0x02
        ? -static_cast<int32_t>(rawBattery)
        : static_cast<int32_t>(rawBattery);
    updateTelemetry(rawPv, signedBattery, rawGrid, rawSoc);
}

void TelemetryMapper::updateTelemetry(uint32_t rawPv, int32_t rawBattery, int32_t rawGrid, uint16_t rawSoc)
{
    // 1. Scale incoming integers to double precision kW metrics (0.001 scale factor)
    m_solarPower = rawPv * 0.001;
    m_batterySoc = static_cast<int>(rawSoc);

    // 2. Separate signs into pure directional components and extract absolute powers
    // Battery: Positive = Charging, Negative = Discharging
    m_isBatteryCharging = (rawBattery >= 0);
    m_batteryPower = std::abs(rawBattery) * 0.001;

    // Grid: Positive = Import, Negative = Export
    m_isGridImporting = (rawGrid >= 0);
    m_gridPower = std::abs(rawGrid) * 0.001;

    // 3. Compute absolute house load baseline calculation layout
    // We normalize terms to the layout: Generation + Discharge - Export
    double batteryDischargekW = (!m_isBatteryCharging) ? m_batteryPower : -m_batteryPower;
    double gridExportkW = (!m_isGridImporting) ? m_gridPower : -m_gridPower;

    m_houseLoad = m_solarPower + batteryDischargekW - gridExportkW;

    // Guard against edge case rounding artifacts dropping below flat zero bounds
    if (m_houseLoad < 0.0) {
        m_houseLoad = 0.0;
    }

    emit telemetryUpdated();
}
