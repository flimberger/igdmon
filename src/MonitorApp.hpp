#ifndef FRITZMON_MONITORAPP_HPP
#define FRITZMON_MONITORAPP_HPP

#include "Settings.hpp"

#include "upnp/DeviceFinder.hpp"

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <QtQuick/QQuickView>

namespace fritzmon {

namespace upnp {

class Service;

} // namespace upnp

class GraphModel;

class MonitorApp : public QObject
{
    Q_OBJECT
public:
    explicit MonitorApp(QObject *parent = nullptr);

private:
    Q_SLOT void onDeviceAdded(upnp::Device *device);
    Q_SLOT void onServiceActionInvoked(const QVariantMap &outputArguments,
                                       const QVariant &returnValue);
    Q_SLOT void onUpdateTimeout();

    enum class AppState {
        Initializing,
        Polling
    } m_appState;
    upnp::DeviceFinder m_deviceFinder;
    GraphModel *m_downstreamData;
    Settings m_settings;
    int m_updatePeriod;
    QTimer m_updateTimer;
    GraphModel *m_upstreamData;
    upnp::Service *m_wanCommonConfigService;
    QQuickView m_view;
};

} // namespace fritzmon

#endif // FRITZMON_MONITORAPP_HPP
