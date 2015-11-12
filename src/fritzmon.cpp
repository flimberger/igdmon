#include "Graph.hpp"
#include "GraphModel.hpp"

#include "upnp/Device.hpp"
#include "upnp/DeviceFinder.hpp"
#include "upnp/Service.hpp"
#include "upnp/ServiceBuilder.hpp"
#include "upnp/StateVariable.hpp"

#include "soap/Request.hpp"

#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtCore/QVariant>

#include <QtGui/QGuiApplication>

#include <QtQml/QQmlContext>

#include <QtQuick/QQuickView>

#include <algorithm>

using namespace fritzmon;

static constexpr auto *APPUI_QML_PATH = "qrc:/fritzmon/qml/appui.qml";
// static constexpr auto *DEVICE_DESCRIPTION_URL = "http://fritz.box:49000/igddesc.xml";
static constexpr auto *DEVICE_DESCRIPTION_URL = "https://fritz.box:49443/igddesc.xml";

static constexpr auto *DOWNSTREAM_DATA_PROPERTY = "downstreamData";
static constexpr auto *UPSTREAM_DATA_PROPERTY = "upstreamData";

static constexpr auto *WAN_DEVICE_TYPE = "urn:schemas-upnp-org:device:WANDevice:1";
static constexpr auto *WAN_COMMON_INTERFACE_CONFIG_SERVICE_TYPE = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";
static constexpr auto *GET_ADDON_INFOS_ACTION_NAME = "GetAddonInfos";
static constexpr auto *NEW_BYTE_SEND_RATE_ARGUMENT = "NewByteSendRate";
static constexpr auto *NEW_BYTE_RECEIVE_RATE_ARGUMENT = "NewByteReceiveRate";

static GraphModel *downstreamData = nullptr;
static GraphModel *upstreamData = nullptr;

void onServiceActionInvoked(const QVariantMap &outputArguments, const QVariant &returnValue)
{
    qDebug() << "::onServiceActionInvoked: return value:" << returnValue;

    if (outputArguments.empty())
        qDebug() << "::onServiceActionInvoked: no output arguments";
    else {
        if (outputArguments.contains(NEW_BYTE_RECEIVE_RATE_ARGUMENT))
            downstreamData->addSample(outputArguments[NEW_BYTE_RECEIVE_RATE_ARGUMENT].toString().toFloat());
        else
            qDebug() << "::onServiceActionInvoked: no argument named"
                     << NEW_BYTE_RECEIVE_RATE_ARGUMENT;
        if (outputArguments.contains(NEW_BYTE_SEND_RATE_ARGUMENT))
            downstreamData->addSample(outputArguments[NEW_BYTE_SEND_RATE_ARGUMENT].toString().toFloat());
        else
            qDebug() << "::onServiceActionInvoked: no argument named"
                     << NEW_BYTE_SEND_RATE_ARGUMENT;
    }
}

void queryDevice(upnp::Device *device)
{
    if (device->type() == WAN_DEVICE_TYPE) {
        auto end = std::end(device->services());
        auto service = std::find_if(std::begin(device->services()), end, [](const std::unique_ptr<upnp::Service> &servicePtr){
            return servicePtr->serviceTypeIdentifier() == WAN_COMMON_INTERFACE_CONFIG_SERVICE_TYPE;
        });

        if (service != end) {
            auto servicePtr = service->get();
            qDebug() << "queryDevice: query" << servicePtr->id();

            QObject::connect(servicePtr, &upnp::Service::actionInvoked, &onServiceActionInvoked);
            if (servicePtr->invokeAction(GET_ADDON_INFOS_ACTION_NAME, QVariantMap()) != upnp::Service::InvokeActionResult::Success)
                qDebug() << "::queryDevice: invocation failed";
        }
    } else
        for (const auto &subdevice : device->children())
            queryDevice(subdevice.get());
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Graph>("Graph", 1, 0, "Graph");
    downstreamData = new GraphModel;
    upstreamData = new GraphModel;

    QQuickView view;
    auto *rootContext = view.rootContext();

    rootContext->setContextProperty(DOWNSTREAM_DATA_PROPERTY, QVariant::fromValue(downstreamData));
    rootContext->setContextProperty(UPSTREAM_DATA_PROPERTY, QVariant::fromValue(upstreamData));
    view.setSource(QUrl(APPUI_QML_PATH));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();

    upnp::DeviceFinder deviceFinder;

    QObject::connect(&deviceFinder, &upnp::DeviceFinder::deviceAdded, &queryDevice);

    deviceFinder.findDevice(QUrl(DEVICE_DESCRIPTION_URL));

    return app.exec();
}
