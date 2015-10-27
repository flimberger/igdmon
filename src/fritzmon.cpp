#include "upnp/Device.hpp"
#include "upnp/DeviceFinder.hpp"
#include "upnp/Service.hpp"

#include <QtCore/QDebug>
#include <QtCore/QUrl>

#include <QtGui/QGuiApplication>

#include <QtQml/QQmlApplicationEngine>

using namespace fritzmon;

static constexpr auto APPUI_QML_PATH = "../assets/qml/appui.qml";
static constexpr auto DEVICE_DESCRIPTION_IURL = "http://fritz.box:49000/igddesc.xml";

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
    QQmlApplicationEngine uiEngine(APPUI_QML_PATH);
    upnp::DeviceFinder deviceFinder;

    auto result = QObject::connect(&deviceFinder, &upnp::DeviceFinder::deviceAdded, &dumpDevice);
    Q_ASSERT(result);
    Q_UNUSED(result);

    deviceFinder.findDevice(QUrl(DEVICE_DESCRIPTION_IURL));

    return app.exec();
}
