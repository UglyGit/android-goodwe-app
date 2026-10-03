#ifndef POLLING_MANAGER_H
#define POLLING_MANAGER_H

#include <QObject>

class QTimer;
class ModbusClient;

class PollingManager final : public QObject
{
    Q_OBJECT

public:
    explicit PollingManager(ModbusClient *client, QObject *parent = nullptr);

    int intervalMs() const;
    bool isRunning() const;
    void setVisible(bool visible);
    void start();
    void stop();

signals:
    void pollRequested();
    void readFailed();

private:
    void poll();

    ModbusClient *m_client;
    QTimer *m_timer;
    bool m_visible = true;
    bool m_requested = false;
};

#endif
