#include "MonitorApp.hpp"

#include "Graph.hpp"
#include "GraphModel.hpp"
#include "upnp/Device.hpp"
#include "upnp/Service.hpp"

#include <QtQml/QQmlContext>

#include <QtQuick/QQuickView>

#include <algorithm>

namespace fritzmon {

static constexpr auto *APPUI_QML_PATH = "qrc:/fritzmon/qml/appui.qml";
static constexpr auto *DEVICE_DESCRIPTION_URL = "https://fritz.box:49443/igddesc.xml";
static constexpr auto *DOWNSTREAM_DATA_PROPERTY = "downstreamData";
static constexpr auto *GET_ADDON_INFOS_ACTION_NAME = "GetAddonInfos";
static constexpr auto *NEW_BYTE_RECEIVE_RATE_ARGUMENT = "NewByteReceiveRate";
static constexpr auto *NEW_BYTE_SEND_RATE_ARGUMENT = "NewByteSendRate";
static constexpr auto *UPSTREAM_DATA_PROPERTY = "upstreamData";
static constexpr auto *WAN_COMMON_INTERFACE_CONFIG_SERVICE_TYPE = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";
static constexpr auto *WAN_DEVICE_TYPE = "urn:schemas-upnp-org:device:WANDevice:1";

MonitorApp::MonitorApp(QObject *parent)
  : QObject(parent),
    m_downstreamData(new GraphModel),
    m_upstreamData(new GraphModel)
{
    connect(&m_deviceFinder, &upnp::DeviceFinder::deviceAdded, this, &MonitorApp::onDeviceAdded);
    m_deviceFinder.findDevice(QUrl(DEVICE_DESCRIPTION_URL));

    QQuickView view;
    auto *rootContext = view.rootContext();

    rootContext->setContextProperty(DOWNSTREAM_DATA_PROPERTY, QVariant::fromValue(m_downstreamData));
    rootContext->setContextProperty(UPSTREAM_DATA_PROPERTY, QVariant::fromValue(m_upstreamData));
    view.setSource(QUrl(APPUI_QML_PATH));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
}

void MonitorApp::onDeviceAdded(upnp::Device *device)
{
    if (device->type() == WAN_DEVICE_TYPE) {
        auto end = std::end(device->services());
        auto service = std::find_if(std::begin(device->services()), end, [](const std::unique_ptr<upnp::Service> &servicePtr){
            return servicePtr->serviceTypeIdentifier() == WAN_COMMON_INTERFACE_CONFIG_SERVICE_TYPE;
        });

        if (service != end) {
            m_wanCommonConfigService = service->get();
            connect(m_wanCommonConfigService, &upnp::Service::actionInvoked,
                    this,                     &MonitorApp::onServiceActionInvoked);
            if (m_wanCommonConfigService->invokeAction(GET_ADDON_INFOS_ACTION_NAME, QVariantMap())
                    != upnp::Service::InvokeActionResult::Success)
                qDebug() << "MonitorApp::onDeviceAdded: invocation failed";
        }
    } else
        for (const auto &subdevice : device->children())
            onDeviceAdded(subdevice.get());
}

void MonitorApp::onServiceActionInvoked(const QVariantMap &outputArguments,
                                        const QVariant &returnValue)
{
    qDebug() << "::onServiceActionInvoked: return value:" << returnValue;

    if (outputArguments.empty())
        qDebug() << "::onServiceActionInvoked: no output arguments";
    else {
        if (outputArguments.contains(NEW_BYTE_RECEIVE_RATE_ARGUMENT))
            m_downstreamData->addSample(outputArguments[NEW_BYTE_RECEIVE_RATE_ARGUMENT].toString()
                                                                                       .toFloat());
        else
            qDebug() << "::onServiceActionInvoked: no argument named"
                     << NEW_BYTE_RECEIVE_RATE_ARGUMENT;
        if (outputArguments.contains(NEW_BYTE_SEND_RATE_ARGUMENT))
            m_downstreamData->addSample(outputArguments[NEW_BYTE_SEND_RATE_ARGUMENT].toString()
                                                                                    .toFloat());
        else
            qDebug() << "::onServiceActionInvoked: no argument named"
                     << NEW_BYTE_SEND_RATE_ARGUMENT;
    }
}

} // namespace fritzmon
