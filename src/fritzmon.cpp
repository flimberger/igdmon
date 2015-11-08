#include "Graph.hpp"
#include "GraphModel.hpp"

#include "upnp/Device.hpp"
#include "upnp/DeviceFinder.hpp"
#include "upnp/Service.hpp"

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

void dumpService(upnp::Service *service)
{
    qDebug() << service->id();
    for (const auto &action : service->actions())
        qDebug() << "\t" << action.name();
}

void dumpDevice(upnp::Device *device)
{
    qDebug() << device->friendlyName();
    for (const auto &service : device->services())
        dumpService(service.get());
    for (const auto &subDevice : device->children())
        dumpDevice(subDevice.get());
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

    auto result = QObject::connect(&deviceFinder, &upnp::DeviceFinder::deviceAdded, &dumpDevice);
    Q_ASSERT(result);
    Q_UNUSED(result);

    deviceFinder.findDevice(QUrl(DEVICE_DESCRIPTION_URL));

    return app.exec();
}
