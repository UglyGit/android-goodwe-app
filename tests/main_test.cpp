#include <QtTest>
#include <QCoreApplication>
#include <QVector>
#include <arpa/inet.h> // Standard Linux networking header containing htons()

#include "inverter_config.h"
#include "modbus_client.h"
#include "telemetry_mapper.h"


class MainTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        QCoreApplication::setOrganizationName("UglyGit");
        QCoreApplication::setApplicationName("GoodWeMonitorTest");
    }

    void testEnvironmentSanity() {
        QCOMPARE(1, 1);
    }

    void testConfigPersistence() {
        InverterConfig config;
        config.clear();
        config.setIpAddress("10.0.2.2");
        config.setPort(5020);

        InverterConfig freshReload;
        QCOMPARE(freshReload.ipAddress(), QString("10.0.2.2"));
        QCOMPARE(freshReload.port(), 5020);
    }

    void testHostValidation() {
        InverterConfig config;
        QVERIFY(config.validateHost("192.168.1.100"));
        QVERIFY(config.validatePort(502));
        QVERIFY(!config.validatePort(0));
        QVERIFY(!config.validateHost("256.0.0.1"));
    }

    void testModbusReadOnlyContract() {
        ModbusClient client;
        QCOMPARE(client.state(), QModbusDevice::UnconnectedState);
        QVERIFY(client.isReadOnlyInterface());
    }

    void testPayloadDecoding() {
        ModbusClient client;
        QVector<quint16> mockRegisters;
        mockRegisters.resize(73);
        mockRegisters.fill(0x0000);

        // 1. Convert our 32-bit host values to Network Byte Order (Big-Endian)
        int32_t hostBatteryPower = -1640;
        int32_t hostGridPower = 820;

        uint32_t netBatteryPower;
        uint32_t netGridPower;

        // Use standard memcpy type-punning to safely pass signed values into htonl()
        std::memcpy(&netBatteryPower, &hostBatteryPower, sizeof(netBatteryPower));
        std::memcpy(&netGridPower, &hostGridPower, sizeof(netGridPower));

        netBatteryPower = htonl(netBatteryPower);
        netGridPower = htonl(netGridPower);

        // 2. Extract upper and lower words from the Big-Endian network chunks
        uint16_t batteryPowerHighWord = static_cast<uint16_t>((netBatteryPower >> 16) & 0xFFFFU);
        uint16_t batteryPowerLowWord  = static_cast<uint16_t>(netBatteryPower & 0xFFFFU);

        uint16_t gridPowerHighWord    = static_cast<uint16_t>((netGridPower >> 16) & 0xFFFFU);
        uint16_t gridPowerLowWord     = static_cast<uint16_t>(netGridPower & 0xFFFFU);

        // 16-bit values use htons() directly
        uint16_t batterySocNet        = htons(82);

        // 3. Load them into our test registers matching GoodWe's register sequence rules
        mockRegisters.replace(0, batteryPowerHighWord);
        mockRegisters.replace(1, batteryPowerLowWord);
        mockRegisters.replace(4, batterySocNet);
        mockRegisters.replace(61, gridPowerHighWord);
        mockRegisters.replace(62, gridPowerLowWord);

        // 4. Execute decode operations
        int32_t batteryPower = client.decodeInt32(mockRegisters, 0);
        uint16_t batterySoc = client.decodeUint16(mockRegisters, 4);
        int32_t gridPower = client.decodeInt32(mockRegisters, 61);

        // 5. Assert mathematical parity
        QCOMPARE(batteryPower, -1640);
        QCOMPARE(batterySoc, static_cast<uint16_t>(82));
        QCOMPARE(gridPower, 820);
    }

    void testTelemetryCalculations() {
        TelemetryMapper mapper;

        // Test Case A: Daytime Generation, Charging Battery, Grid Exporting
        // PV: 5.0 kW (5000), Battery: +2.0 kW Charging (2000), Grid: -1.0 kW Exporting (-1000)
        // Correct Energy Balance: House Load = 5.0 (Gen) - 2.0 (To Battery) - 1.0 (To Grid) = 2.0 kW
        mapper.processRawTelemetry(5000, 2000, -1000, 85);
        QCOMPARE(mapper.solarPower(), 5.0);
        QCOMPARE(mapper.batteryPower(), 2.0);
        QVERIFY(mapper.isBatteryCharging() == true);
        QCOMPARE(mapper.gridPower(), 1.0);
        QVERIFY(mapper.isGridImporting() == false);
        QCOMPARE(mapper.houseLoad(), 2.0); // Fixed assertion value to match physical law

        // Test Case B: Nighttime No Generation, Discharging Battery, Grid Importing
        // PV: 0.0 kW (0), Battery: -1.5 kW Discharging (-1500), Grid: +2.0 kW Importing (2000)
        // Correct Energy Balance: House Load = 0.0 (Gen) + 1.5 (From Battery) + 2.0 (From Grid) = 3.5 kW
        mapper.processRawTelemetry(0, -1500, 2000, 42);
        QCOMPARE(mapper.solarPower(), 0.0);
        QCOMPARE(mapper.batteryPower(), 1.5);
        QVERIFY(mapper.isBatteryCharging() == false);
        QCOMPARE(mapper.gridPower(), 2.0);
        QVERIFY(mapper.isGridImporting() == true);
        QCOMPARE(mapper.houseLoad(), 3.5); // Fixed assertion value to match physical law
    }

    void cleanupTestCase() {
        InverterConfig config;
        config.clear();
    }
};

QTEST_MAIN(MainTest)
#include "main_test.moc"