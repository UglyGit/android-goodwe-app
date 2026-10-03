#include "polling_manager.h"

#include "modbus_client.h"

#include <QTimer>

PollingManager::PollingManager(ModbusClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(5000);
    connect(m_timer, &QTimer::timeout, this, &PollingManager::poll);
    connect(m_client, &ModbusClient::readFailed, this, &PollingManager::readFailed);
}

int PollingManager::intervalMs() const
{
    return m_timer->interval();
}

bool PollingManager::isRunning() const
{
    return m_timer->isActive();
}

void PollingManager::setVisible(bool visible)
{
    m_visible = visible;
    if (!m_visible) {
        m_timer->stop();
    } else if (m_requested) {
        m_timer->start();
        poll();
    }
}

void PollingManager::start()
{
    if (!m_visible || isRunning()) {
        return;
    }
    m_requested = true;
    m_timer->start();
    poll();
}

void PollingManager::stop()
{
    m_requested = false;
    m_timer->stop();
}

void PollingManager::poll()
{
    emit pollRequested();
    m_client->readTelemetryBlock();
}
