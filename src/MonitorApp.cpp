/*
 * Copyright (c) 2016 Florian Limberger <flo@snakeoilproductions.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MonitorApp.hpp"

#include "Graph.hpp"
#include "GraphModel.hpp"
#include "upnp/Device.hpp"
#include "upnp/Service.hpp"

#include <QtCore/QCoreApplication>

#include <QtQml/QQmlContext>

#include <algorithm>

namespace fritzmon {

static constexpr auto *ORG_NAME = "Purple Kraken Software";
static constexpr auto *ORG_DOMAIN = "purplekraken.com";
static constexpr auto *APP_NAME = "fritzmon";
static constexpr auto *APPUI_QML_PATH = "qrc:/fritzmon/qml/appui.qml";
static constexpr auto DEFAULT_UPDATE_PERIOD = 2500; //< update period in ms
static constexpr auto *DEVICE_DEFAULT_HOST = "fritz.box";
static constexpr auto DEVICE_DEFAULT_PORT = 49000;
static constexpr auto DEVICE_DEFAULT_ENCRYPTION = false;
static constexpr auto *DEVICE_DESCRIPTION_DOCUMENT = "/igddesc.xml";
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
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain(ORG_DOMAIN);
    QCoreApplication::setApplicationName(APP_NAME);

    m_settings.readConfiguration();
    if (m_settings.host().isEmpty())
        m_settings.setHost(DEVICE_DEFAULT_HOST);
    if (m_settings.port() == -1)
        m_settings.setPort(DEVICE_DEFAULT_PORT);
    if (!m_settings.useSSL())
        m_settings.setUseSSL(DEVICE_DEFAULT_ENCRYPTION);

    auto deviceDescriptionURL = m_settings.deviceURL();

    deviceDescriptionURL.setPath(DEVICE_DESCRIPTION_DOCUMENT);
    connect(&m_deviceFinder, &upnp::DeviceFinder::deviceAdded, this, &MonitorApp::onDeviceAdded);
    m_deviceFinder.findDevice(deviceDescriptionURL);

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
