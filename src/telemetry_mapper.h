#ifndef TELEMETRY_MAPPER_H
#define TELEMETRY_MAPPER_H

#include <QObject>

class TelemetryMapper : public QObject
{
    Q_OBJECT

    // Decoupled properties exposed seamlessly to QML Presentation engines
    Q_PROPERTY(double solarPower READ solarPower NOTIFY telemetryUpdated)
    Q_PROPERTY(double batteryPower READ batteryPower NOTIFY telemetryUpdated)
    Q_PROPERTY(bool isBatteryCharging READ isBatteryCharging NOTIFY telemetryUpdated)
    Q_PROPERTY(double gridPower READ gridPower NOTIFY telemetryUpdated)
    Q_PROPERTY(bool isGridImporting READ isGridImporting NOTIFY telemetryUpdated)
    Q_PROPERTY(double houseLoad READ houseLoad NOTIFY telemetryUpdated)
    Q_PROPERTY(int batterySoc READ batterySoc NOTIFY telemetryUpdated)

public:
    explicit TelemetryMapper(QObject *parent = nullptr);

    // Primary entry pipeline to feed processed Modbus register values into the mapper
    void processRawTelemetry(uint32_t rawPv, int32_t rawBattery, int32_t rawGrid, uint16_t rawSoc);

    // Property Getter accessors
    double solarPower() const { return m_solarPower; }
    double batteryPower() const { return m_batteryPower; }
    bool isBatteryCharging() const { return m_isBatteryCharging; }
    double gridPower() const { return m_gridPower; }
    bool isGridImporting() const { return m_isGridImporting; }
    double houseLoad() const { return m_houseLoad; }
    int batterySoc() const { return m_batterySoc; }

signals:
    void telemetryUpdated();

private:
    double m_solarPower = 0.0;
    double m_batteryPower = 0.0;
    bool m_isBatteryCharging = false;
    double m_gridPower = 0.0;
    bool m_isGridImporting = false;
    double m_houseLoad = 0.0;
    int m_batterySoc = 0;
};

#endif // TELEMETRY_MAPPER_H
