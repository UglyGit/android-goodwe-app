#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H

#include <QObject>
#include <QModbusTcpClient>
#include <QVector>
#include <cstdint>

class ModbusClient : public QObject
{
    Q_OBJECT

public:
    explicit ModbusClient(QObject *parent = nullptr);
    ~ModbusClient();

    void connectToInverter(const QString &ip, int port);
    void disconnectFromInverter();

    QModbusDevice::State state() const;
    bool isReadOnlyInterface() const;

    // New Decoding Utilities (Issue 09)
    uint16_t decodeUint16(const QVector<quint16> &buffer, int offset) const;
    uint32_t decodeUint32(const QVector<quint16> &buffer, int offset) const;
    int32_t decodeInt32(const QVector<quint16> &buffer, int offset) const;

signals:
    void connectionStateChanged(QModbusDevice::State state);
    void blockReadCompleted(const QVector<quint16> &values);
    void readFailed();

private:
    QModbusTcpClient *m_modbusDevice;
};

#endif // MODBUS_CLIENT_H
