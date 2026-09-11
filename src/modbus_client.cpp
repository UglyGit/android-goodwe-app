#include "modbus_client.h"
#include <QVariant>
#include <arpa/inet.h> // Linux networking header for ntohl and ntohs

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
    
    // Convert 16-bit network word directly to host format
    return ntohs(buffer[offset]);
}

uint32_t ModbusClient::decodeUint32(const QVector<quint16> &buffer, int offset) const
{
    if (offset < 0 || (offset + 1) >= buffer.size()) return 0;
    
    // 1. Reconstruct the 32-bit network block exactly as it came off the wire
    uint32_t netData32 = (static_cast<uint32_t>(buffer[offset]) << 16) | buffer[offset + 1];
    
    // 2. Convert the entire 32-bit block from Network to Host order natively
    return ntohl(netData32);
}

int32_t ModbusClient::decodeInt32(const QVector<quint16> &buffer, int offset) const
{
    uint32_t hostData32 = decodeUint32(buffer, offset);
    int32_t result;
    std::memcpy(&result, &hostData32, sizeof(result));
    return result;
}
