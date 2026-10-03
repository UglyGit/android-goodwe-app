#include "app_state_orchestrator.h"

AppStateOrchestrator::AppStateOrchestrator(QObject *parent)
    : QObject(parent)
{
}

AppStateOrchestrator::View AppStateOrchestrator::view() const { return m_view; }
bool AppStateOrchestrator::isOffline() const { return m_offline; }
bool AppStateOrchestrator::isStale() const { return m_stale; }
bool AppStateOrchestrator::dashboardValuesAvailable() const
{
    return m_view == DashboardView && !m_offline && !m_stale;
}

void AppStateOrchestrator::startDashboard()
{
    if (m_view == DashboardView) return;
    m_view = DashboardView;
    emit viewChanged();
    emit connectionStateChanged();
}

void AppStateOrchestrator::openSettings()
{
    if (m_view == SetupView) return;
    m_view = SetupView;
    emit viewChanged();
}

void AppStateOrchestrator::markConnectionLost()
{
    if (m_offline && m_stale) return;
    m_offline = true;
    m_stale = true;
    emit connectionStateChanged();
}

void AppStateOrchestrator::markConnectionRestored()
{
    if (!m_offline && !m_stale) return;
    m_offline = false;
    m_stale = false;
    emit connectionStateChanged();
}
