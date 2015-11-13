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
static constexpr auto *DOWNSTREAM_GRAPH = "downstreamGraph";
static constexpr auto *GET_ADDON_INFOS_ACTION_NAME = "GetAddonInfos";
static constexpr auto *GET_COMMON_LINK_PROPERTIES_ACTION_NAME = "GetCommonLinkProperties";
static constexpr auto *NEW_BYTE_RECEIVE_RATE_ARGUMENT = "NewByteReceiveRate";
static constexpr auto *NEW_BYTE_SEND_RATE_ARGUMENT = "NewByteSendRate";
static constexpr auto *NEW_LAYER_1_DOWNSTREAM_MAX_BIT_RATE = "NewLayer1DownstreamMaxBitRate";
static constexpr auto *NEW_LAYER_1_UPSTREAM_MAX_BIT_RATE = "NewLayer1UpstreamMaxBitRate";
static constexpr auto *UPSTREAM_DATA_PROPERTY = "upstreamData";
static constexpr auto *UPSTREAM_GRAPH = "upstreamGraph";
static constexpr auto *WAN_COMMON_INTERFACE_CONFIG_SERVICE_TYPE = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";
static constexpr auto *WAN_DEVICE_TYPE = "urn:schemas-upnp-org:device:WANDevice:1";

MonitorApp::MonitorApp(QObject *parent)
  : QObject(parent),
    m_appState(AppState::Initializing),
    m_downstreamData(new GraphModel),
    m_updatePeriod(DEFAULT_UPDATE_PERIOD),
    m_upstreamData(new GraphModel)
{
    connect(&m_deviceFinder, &upnp::DeviceFinder::deviceAdded, this, &MonitorApp::onDeviceAdded);
    m_deviceFinder.findDevice(QUrl(DEVICE_DESCRIPTION_URL));

    auto *rootContext = m_view.rootContext();

    rootContext->setContextProperty(DOWNSTREAM_DATA_PROPERTY, QVariant::fromValue(m_downstreamData));
    rootContext->setContextProperty(UPSTREAM_DATA_PROPERTY, QVariant::fromValue(m_upstreamData));
    m_view.setResizeMode(QQuickView::SizeRootObjectToView);
    m_view.setSource(QUrl(APPUI_QML_PATH));
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
            m_wanCommonConfigService->invokeAction(GET_COMMON_LINK_PROPERTIES_ACTION_NAME,
                                                   QVariantMap());
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
        switch (m_appState) {
        case AppState::Initializing:
            {
            auto rootObject = m_view.rootObject();

            if (outputArguments.contains(NEW_LAYER_1_DOWNSTREAM_MAX_BIT_RATE)) {
                auto value = outputArguments[NEW_LAYER_1_DOWNSTREAM_MAX_BIT_RATE].toString()
                                                                                 .toFloat();

                qDebug() << "MonitorApp::onServiceActionInvoked: max downstream bit rate:" << value;

                auto *graph = rootObject->findChild<Graph *>(DOWNSTREAM_GRAPH);

                if (graph)
                    graph->setUpperBound(value / 1024.0f); // convert bit to kbit
                else
                    qDebug() << "MonitorApp::onServiceActionInvoked: failed to find"
                             << DOWNSTREAM_GRAPH;
            } else
                qDebug() << "MonitorApp::onServiceActionInvoked: no argument named"
                         << NEW_LAYER_1_DOWNSTREAM_MAX_BIT_RATE;
            if (outputArguments.contains(NEW_LAYER_1_UPSTREAM_MAX_BIT_RATE)) {
                auto value = outputArguments[NEW_LAYER_1_UPSTREAM_MAX_BIT_RATE].toString()
                                                                               .toFloat();

                qDebug() << "MonitorApp::onServiceActionInvoked: max upstream bit rate:" << value;

                auto *graph = rootObject->findChild<Graph *>(UPSTREAM_GRAPH);

                if (graph)
                    graph->setUpperBound(value / 1024.0f); // convert bit to kbit
                else
                    qDebug() << "MonitorApp::onServiceActionInvoked: failed to find"
                             << UPSTREAM_GRAPH;
            } else
                qDebug() << "MonitorApp::onServiceActionInvoked: no argument named"
                         << NEW_LAYER_1_DOWNSTREAM_MAX_BIT_RATE;
            m_appState = AppState::Polling;
            break;
            }
        case AppState::Polling:
            if (outputArguments.contains(NEW_BYTE_RECEIVE_RATE_ARGUMENT)) {
                auto value = outputArguments[NEW_BYTE_RECEIVE_RATE_ARGUMENT].toString().toFloat();

                m_downstreamData->addSample(value * 8.0f / 1024.0f); // convert B to kbit
                qDebug() << "MonitorApp::onServiceActionInvoked: downstream byte rate:" << value;
            } else
                qDebug() << "MonitorApp::onServiceActionInvoked: no argument named"
                         << NEW_BYTE_RECEIVE_RATE_ARGUMENT;
            if (outputArguments.contains(NEW_BYTE_SEND_RATE_ARGUMENT)) {
                auto value = outputArguments[NEW_BYTE_SEND_RATE_ARGUMENT].toString().toFloat();

                m_upstreamData->addSample(value * 8.0f / 1024.0f); // convert B to kbit
                qDebug() << "MonitorApp::onServiceActionInvoked: upstream byte rate:" << value;
            } else
                qDebug() << "MonitorApp::onServiceActionInvoked: no argument named"
                         << NEW_BYTE_SEND_RATE_ARGUMENT;
            break;
        }
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
