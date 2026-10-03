#include <QGuiApplication>
#include <QQuickView>
#include <QUrl>
#include <QQmlContext>
#include "modbus_client.h"
#include "polling_manager.h"
#include "telemetry_mapper.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    ModbusClient modbusClient;
    TelemetryMapper telemetry;
    PollingManager polling(&modbusClient);
    view.rootContext()->setContextProperty("modbusClient", &modbusClient);
    view.rootContext()->setContextProperty("telemetry", &telemetry);
    QObject::connect(&modbusClient, &ModbusClient::blockReadCompleted, &telemetry,
        [&telemetry, &modbusClient](const QVector<quint16> &values) {
            telemetry.processRawTelemetry(
                modbusClient.decodeUint32(values, 190),
                modbusClient.decodeUint32(values, 71),
                modbusClient.decodeUint16(values, 73),
                static_cast<int32_t>(modbusClient.decodeUint32(values, 26)),
                modbusClient.decodeUint16(values, 1896));
        });
    QObject::connect(&modbusClient, &ModbusClient::connectionStateChanged,
        [&polling](QModbusDevice::State state) {
            if (state == QModbusDevice::ConnectedState) polling.start();
            else polling.stop();
        });
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl(QStringLiteral("qrc:/GoodWeApp/src/MainForm.qml")));
    if (view.status() == QQuickView::Error) return 1;
    view.show();
    return app.exec();
}
