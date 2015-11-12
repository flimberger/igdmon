#include "Graph.hpp"
#include "MonitorApp.hpp"

#include <QtGui/QGuiApplication>

int main(int argc, char *argv[])
{
    qmlRegisterType<fritzmon::Graph>("Graph", 1, 0, "Graph");

    QGuiApplication app(argc, argv);
    fritzmon::MonitorApp monitorApp;

    return app.exec();
}
