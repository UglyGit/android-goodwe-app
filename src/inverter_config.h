#ifndef INVERTER_CONFIG_H
#define INVERTER_CONFIG_H

#include <QObject>
#include <QString>
#include <QSettings>

class InverterConfig : public QObject
{
    Q_OBJECT

public:
    explicit InverterConfig(QObject *parent = nullptr);

    // Public Getters
    QString ipAddress() const;
    int port() const;

    // Public Setters
    void setIpAddress(const QString &ip);
    void setPort(int port);

    // New Validation Methods (Issue 05)
    bool validateHost(const QString &host) const;
    bool validatePort(int port) const;

    // Lifecycle helper
    void clear();

private:
    QSettings m_settings;
};

#endif // INVERTER_CONFIG_H
