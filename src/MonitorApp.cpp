#include "MonitorApp.hpp"

#include "Graph.hpp"
#include "GraphModel.hpp"
#include "upnp/Device.hpp"
#include "upnp/Service.hpp"

#include <QtQml/QQmlContext>

#include <algorithm>

namespace fritzmon {

static constexpr auto *APPUI_QML_PATH = "qrc:/fritzmon/qml/appui.qml";
static constexpr auto DEFAULT_UPDATE_PERIOD = 2500; //< update period in ms
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
    m_updatePeriod(DEFAULT_UPDATE_PERIOD),
    m_upstreamData(new GraphModel)
{
    connect(&m_deviceFinder, &upnp::DeviceFinder::deviceAdded, this, &MonitorApp::onDeviceAdded);
    m_deviceFinder.findDevice(QUrl(DEVICE_DESCRIPTION_URL));

    auto *rootContext = m_view.rootContext();

    rootContext->setContextProperty(DOWNSTREAM_DATA_PROPERTY, QVariant::fromValue(m_downstreamData));
    rootContext->setContextProperty(UPSTREAM_DATA_PROPERTY, QVariant::fromValue(m_upstreamData));
    m_view.setSource(QUrl(APPUI_QML_PATH));
    m_view.setResizeMode(QQuickView::SizeRootObjectToView);
    m_view.show();
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
            connect(&m_updateTimer, &QTimer::timeout, this, &MonitorApp::onUpdateTimeout);
            onUpdateTimeout();
            m_updateTimer.start(m_updatePeriod);
        }
    } else
        for (const auto &subdevice : device->children())
            onDeviceAdded(subdevice.get());
}

void MonitorApp::onServiceActionInvoked(const QVariantMap &outputArguments,
                                        const QVariant &returnValue)
{
    qDebug() << "MonitorApp::onServiceActionInvoked: return value:" << returnValue;

    if (outputArguments.empty())
        qDebug() << "MonitorApp::onServiceActionInvoked: no output arguments";
    else {
        if (outputArguments.contains(NEW_BYTE_RECEIVE_RATE_ARGUMENT)) {
            auto value = outputArguments[NEW_BYTE_RECEIVE_RATE_ARGUMENT].toString().toFloat();

            m_downstreamData->addSample(value);
            qDebug() << "MonitorApp::onServiceActionInvoked: downstream=" << value;
        } else
            qDebug() << "MonitorApp::onServiceActionInvoked: no argument named"
                     << NEW_BYTE_RECEIVE_RATE_ARGUMENT;
        if (outputArguments.contains(NEW_BYTE_SEND_RATE_ARGUMENT)) {
            auto value = outputArguments[NEW_BYTE_SEND_RATE_ARGUMENT].toString().toFloat();

            m_downstreamData->addSample(value);
            qDebug() << "MonitorApp::onServiceActionInvoked: downstream=" << value;
        } else
            qDebug() << "MonitorApp::onServiceActionInvoked: no argument named"
                     << NEW_BYTE_SEND_RATE_ARGUMENT;
    }
}

void MonitorApp::onUpdateTimeout()
{
    static auto noArg = QVariantMap();

    auto result = m_wanCommonConfigService->invokeAction(GET_ADDON_INFOS_ACTION_NAME, noArg);

    if (result != upnp::Service::InvokeActionResult::Success)
        auto resultString = QString();

        switch (result) {
        case upnp::Service::InvokeActionResult::Success:
            break;
        case upnp::Service::InvokeActionResult::InvalidAction:
            qDebug() << "MonitorApp::onDeviceAdded: invalid action";
            break;
        case upnp::Service::InvokeActionResult::InvocationFailed:
            qDebug() << "MonitorApp::onDeviceAdded: invocation failed";
            break;
        case upnp::Service::InvokeActionResult::PendingAction:
            qDebug() << "MonitorApp::onDeviceAdded: action pending";
            break;
        }
}

} // namespace fritzmon
