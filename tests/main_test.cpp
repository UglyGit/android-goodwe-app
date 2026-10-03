#include <QtTest>
#include <QCoreApplication>
#include <QVector>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>

#include "inverter_config.h"
#include "modbus_client.h"
#include "telemetry_mapper.h"
#include "polling_manager.h"
#include "app_state_orchestrator.h"
#include "history_repository.h"


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

    void testTelemetryBlockReadRequiresConnection() {
        ModbusClient client;
        QSignalSpy failedSpy(&client, &ModbusClient::readFailed);

        QVERIFY(!client.readTelemetryBlock());
        QCOMPARE(failedSpy.count(), 1);
    }

    void testPayloadDecoding() {
        ModbusClient client;
        QVector<quint16> mockRegisters;
        mockRegisters.resize(73);
        mockRegisters.fill(0x0000);

        // Modbus registers are already exposed as numeric 16-bit words by Qt.
        mockRegisters.replace(0, 0xFFFF);
        mockRegisters.replace(1, 0xF998); // -1640
        mockRegisters.replace(4, 82);
        mockRegisters.replace(61, 0x0000);
        mockRegisters.replace(62, 0x0334); // 820

        // 4. Execute decode operations
        int32_t batteryPower = client.decodeInt32(mockRegisters, 0);
        uint16_t batterySoc = client.decodeUint16(mockRegisters, 4);
        int32_t gridPower = client.decodeInt32(mockRegisters, 61);

        // 5. Assert mathematical parity
        QCOMPARE(batteryPower, -1640);
        QCOMPARE(batterySoc, static_cast<uint16_t>(82));
        QCOMPARE(gridPower, 820);
        QCOMPARE(client.decodeUint32({0x1234, 0x5678}, 0), 0x12345678U);
        QCOMPARE(client.decodeUint16({0x1234}, 0), 0x1234U);
        QCOMPARE(client.decodeUint32({0x1234}, 0), 0U);
        QCOMPARE(client.decodeUint16({}, 0), 0U);
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

    void testTelemetryDeviceBatteryMode() {
        TelemetryMapper mapper;

        // Device exposes battery magnitude separately; mode supplies direction.
        mapper.processRawTelemetry(4000, 1000, 0x03, -500, 50);
        QCOMPARE(mapper.batteryPower(), 1.0);
        QVERIFY(mapper.isBatteryCharging());
        QCOMPARE(mapper.houseLoad(), 2.5);

        mapper.processRawTelemetry(0, 1500, 0x02, 2000, 50);
        QCOMPARE(mapper.batteryPower(), 1.5);
        QVERIFY(!mapper.isBatteryCharging());
        QCOMPARE(mapper.houseLoad(), 3.5);
    }

    void testPollingManagerLifecycle() {
        ModbusClient client;
        PollingManager manager(&client);
        QSignalSpy pollSpy(&manager, &PollingManager::pollRequested);

        QCOMPARE(manager.intervalMs(), 5000);
        QVERIFY(!manager.isRunning());

        manager.start();
        QVERIFY(manager.isRunning());
        QCOMPARE(pollSpy.count(), 1);

        manager.setVisible(false);
        QVERIFY(!manager.isRunning());
        manager.setVisible(true);
        QVERIFY(manager.isRunning());
        QCOMPARE(pollSpy.count(), 2);

        manager.stop();
        QVERIFY(!manager.isRunning());
    }

    void testAppStateOrchestratorLifecycle() {
        AppStateOrchestrator state;
        QCOMPARE(state.view(), AppStateOrchestrator::SetupView);
        QVERIFY(!state.dashboardValuesAvailable());

        state.startDashboard();
        QCOMPARE(state.view(), AppStateOrchestrator::DashboardView);
        QVERIFY(state.dashboardValuesAvailable());

        state.markConnectionLost();
        QVERIFY(state.isOffline());
        QVERIFY(state.isStale());
        QVERIFY(!state.dashboardValuesAvailable());

        state.markConnectionRestored();
        QVERIFY(!state.isOffline());
        QVERIFY(!state.isStale());
        QVERIFY(state.dashboardValuesAvailable());

        state.openSettings();
        QCOMPARE(state.view(), AppStateOrchestrator::SetupView);
    }

    void testHistoryPersistsSamplesAndKeepsGaps() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString path = tempDir.filePath("history.json");
        HistoryRepository history(path);

        const QDateTime first = QDateTime::fromSecsSinceEpoch(1000, Qt::UTC);
        const QDateTime third = first.addSecs(600);
        QVERIFY(history.addSample(first, 76));
        QVERIFY(history.addSample(third, 74));

        HistoryRepository reloaded(path);
        const QVector<HistorySample> samples = reloaded.samples(first, third);
        QCOMPARE(samples.size(), 2);
        QCOMPARE(samples.at(0).timestamp, first);
        QCOMPARE(samples.at(0).soc, 76);
        QCOMPARE(samples.at(1).timestamp, third);
        QCOMPARE(samples.at(1).soc, 74);
    }

    void testHistoryRetainsOnly24Hours() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        HistoryRepository history(tempDir.filePath("history.json"));
        const QDateTime now = QDateTime::fromSecsSinceEpoch(200000, Qt::UTC);

        QVERIFY(history.addSample(now.addSecs(-(24 * 60 * 60 + 1)), 50));
        QVERIFY(history.addSample(now, 51));

        const QVector<HistorySample> samples = history.samples(now.addSecs(-24 * 60 * 60), now);
        QCOMPARE(samples.size(), 1);
        QCOMPARE(samples.first().soc, 51);
    }

    void testDashboardCanvasDeclaresLiveMetricsAndHistory() {
        QFile dashboard("src/DashboardView.qml");
        QVERIFY(dashboard.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString qml = QString::fromUtf8(dashboard.readAll());
        QVERIFY(qml.contains("solarPower"));
        QVERIFY(qml.contains("batterySoc"));
        QVERIFY(qml.contains("Canvas"));
    }

    void cleanupTestCase() {
        InverterConfig config;
        config.clear();
    }
};

QTEST_MAIN(MainTest)
#include "main_test.moc"
