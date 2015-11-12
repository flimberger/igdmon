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

using namespace fritzmon;

static constexpr auto *APPUI_QML_PATH = "qrc:/fritzmon/qml/appui.qml";
// static constexpr auto *DEVICE_DESCRIPTION_URL = "http://fritz.box:49000/igddesc.xml";
static constexpr auto *DEVICE_DESCRIPTION_URL = "https://fritz.box:49443/igddesc.xml";
static constexpr auto *GET_ADDON_INFOS_ACTION_NAME = "GetAddonInfos";

void dumpService(upnp::Service *service)
{
    qDebug() << "\t" << service->id() << service->serviceTypeIdentifier();
    for (const auto &action : service->actions())
        qDebug() << "\t\taction:" << action.name();
    for (const auto &variable : service->stateVariables())
        qDebug() << "\t\tstate variable:" << variable.name();
}

void dumpDevice(upnp::Device *device)
{
    qDebug() << device->friendlyName() << device->type();
    for (const auto &service : device->services())
        dumpService(service.get());
    for (const auto &subDevice : device->children())
        dumpDevice(subDevice.get());
}

void printResponse(const QVariantMap &outputArguments, const QVariant &returnValue)
{
    qDebug() << "::printResponse: return value:" << returnValue;

    if (!outputArguments.empty()) {
        const auto end = std::cend(outputArguments);

        for (auto i = std::cbegin(outputArguments); i != end; ++i)
            qDebug() << "::printResponse:" << i.key() << ":" << i.value().toString();
    }
}

void queryDevice(upnp::Device *device)
{
    if (device->type() == "urn:schemas-upnp-org:device:WANDevice:1") {
        for (const auto &service : device->services())
            if (service->serviceTypeIdentifier()
                    == "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1") {
                qDebug() << "queryDevice: query" << service->id();

                QObject::connect(service.get(), &upnp::Service::actionInvoked, &printResponse);
                if (service->invokeAction(GET_ADDON_INFOS_ACTION_NAME, QVariantMap()) != upnp::Service::InvokeActionResult::Success)
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

    auto *downstreamData = new GraphModel;
    auto *upstreamData = new GraphModel;
    QQuickView view;
    auto *rootContext = view.rootContext();

    rootContext->setContextProperty("downstreamData", QVariant::fromValue(downstreamData));
    rootContext->setContextProperty("upstreamData", QVariant::fromValue(upstreamData));
    view.setSource(QUrl(APPUI_QML_PATH));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();

    upnp::DeviceFinder deviceFinder;

    QObject::connect(&deviceFinder, &upnp::DeviceFinder::deviceAdded, &dumpDevice);
    QObject::connect(&deviceFinder, &upnp::DeviceFinder::deviceAdded, &queryDevice);

    deviceFinder.findDevice(QUrl(DEVICE_DESCRIPTION_URL));

    return app.exec();
}
