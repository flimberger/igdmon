#ifndef FRITZMON_MONITORAPP_HPP
#define FRITZMON_MONITORAPP_HPP

#include "upnp/DeviceFinder.hpp"

#include <QtCore/QObject>
#include <QtCore/QTimer>

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

    upnp::DeviceFinder m_deviceFinder;
    GraphModel *m_downstreamData;
    QTimer m_updateTimer;
    GraphModel *m_upstreamData;
    upnp::Service *m_wanCommonConfigService;
};

} // namespace fritzmon

#endif // FRITZMON_MONITORAPP_HPP
