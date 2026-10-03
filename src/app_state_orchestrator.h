#ifndef APP_STATE_ORCHESTRATOR_H
#define APP_STATE_ORCHESTRATOR_H

#include <QObject>

class AppStateOrchestrator final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(View view READ view NOTIFY viewChanged)
    Q_PROPERTY(bool offline READ isOffline NOTIFY connectionStateChanged)
    Q_PROPERTY(bool stale READ isStale NOTIFY connectionStateChanged)
    Q_PROPERTY(bool dashboardValuesAvailable READ dashboardValuesAvailable NOTIFY connectionStateChanged)

public:
    enum View { SetupView, DashboardView };
    Q_ENUM(View)

    explicit AppStateOrchestrator(QObject *parent = nullptr);

    View view() const;
    bool isOffline() const;
    bool isStale() const;
    bool dashboardValuesAvailable() const;

    Q_INVOKABLE void startDashboard();
    Q_INVOKABLE void openSettings();
    void markConnectionLost();
    void markConnectionRestored();

signals:
    void viewChanged();
    void connectionStateChanged();

private:
    View m_view = SetupView;
    bool m_offline = false;
    bool m_stale = false;
};

#endif
