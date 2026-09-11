#include "inverter_config.h"
#include <QRegularExpression>

InverterConfig::InverterConfig(QObject *parent)
    : QObject(parent)
{
}

QString InverterConfig::ipAddress() const
{
    return m_settings.value("inverter/ip", "").toString();
}

int InverterConfig::port() const
{
    return m_settings.value("inverter/port", 502).toInt();
}

void InverterConfig::setIpAddress(const QString &ip)
{
    m_settings.setValue("inverter/ip", ip);
}

void InverterConfig::setPort(int port)
{
    m_settings.setValue("inverter/port", port);
}

bool InverterConfig::validateHost(const QString &host) const
{
    // Strict IPv4 validation regex matching standard octet boundaries (0-255)
    static const QRegularExpression ipRegex(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    return ipRegex.match(host).hasMatch();
}

bool InverterConfig::validatePort(int port) const
{
    // Enforce valid TCP socket boundaries strictly between 1 and 65535
    return (port >= 1 && port <= 65535);
}

void InverterConfig::clear()
{
    m_settings.clear();
}
