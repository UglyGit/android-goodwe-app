#include "modbus_client.h"
#include <QVariant>
#include <QModbusDataUnit>
#include <QModbusReply>

ModbusClient::ModbusClient(QObject *parent)
    : QObject(parent)
    , m_modbusDevice(new QModbusTcpClient(this))
{
    connect(m_modbusDevice, &QModbusDevice::stateChanged, this, &ModbusClient::connectionStateChanged);
}

ModbusClient::~ModbusClient()
{
    disconnectFromInverter();
}

void ModbusClient::connectToInverter(const QString &ip, int port)
{
    if (m_modbusDevice->state() == QModbusDevice::ConnectedState) {
        return;
    }

    m_modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QVariant(ip));
    m_modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, QVariant(port));
    m_modbusDevice->connectDevice();
}

void ModbusClient::disconnectFromInverter()
{
    if (m_modbusDevice->state() != QModbusDevice::UnconnectedState) {
        m_modbusDevice->disconnectDevice();
    }
}

bool ModbusClient::readTelemetryBlock(int serverAddress)
{
    if (m_modbusDevice->state() != QModbusDevice::ConnectedState
        || serverAddress < 1 || serverAddress > 247) {
        emit readFailed();
        return false;
    }

    // QModbusDataUnit addresses are zero-based.  The documented GoodWe
    // range 35111..35183 therefore starts at 35110 and contains 73 words.
    m_pendingTelemetry.fill(0, 1898);
    m_pendingReads = 0;
    const QVector<QPair<int, int>> blocks{{35110, 75}, {35300, 2}, {37006, 1}};
    for (const auto &[address, count] : blocks) {
        QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, address, count);
        QModbusReply *reply = m_modbusDevice->sendReadRequest(request, serverAddress);
        if (reply == nullptr) { emit readFailed(); return false; }
        ++m_pendingReads;
        connect(reply, &QModbusReply::finished, this, [this, reply, address]() {
            if (reply->error() != QModbusDevice::NoError) {
                m_pendingReads = 0;
                emit readFailed();
            } else {
                const QVector<quint16> values = reply->result().values();
                const int offset = address - 35110;
                for (int i = 0; i < values.size() && offset + i < m_pendingTelemetry.size(); ++i)
                    m_pendingTelemetry[offset + i] = values.at(i);
                if (--m_pendingReads == 0) emit blockReadCompleted(m_pendingTelemetry);
            }
            reply->deleteLater();
        });
    }
    return true;
}

QModbusDevice::State ModbusClient::state() const
{
    return m_modbusDevice->state();
}

bool ModbusClient::isReadOnlyInterface() const
{
    return true;
}

uint16_t ModbusClient::decodeUint16(const QVector<quint16> &buffer, int offset) const
{
    if (offset < 0 || offset >= buffer.size()) return 0;
    
    return buffer[offset];
}

uint32_t ModbusClient::decodeUint32(const QVector<quint16> &buffer, int offset) const
{
    if (offset < 0 || (offset + 1) >= buffer.size()) return 0;
    
    return (static_cast<uint32_t>(buffer[offset]) << 16) | buffer[offset + 1];
}

int32_t ModbusClient::decodeInt32(const QVector<quint16> &buffer, int offset) const
{
    return static_cast<int32_t>(decodeUint32(buffer, offset));
}
