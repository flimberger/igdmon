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

#ifndef FRITZMON_MONITORAPP_HPP
#define FRITZMON_MONITORAPP_HPP

#include "Settings.hpp"

#include "upnp/DeviceFinder.hpp"

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <QtQuick/QQuickView>

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
    Q_SLOT void onUpdateTimeout();

    enum class AppState {
        Initializing,
        Polling
    } m_appState;
    upnp::DeviceFinder m_deviceFinder;
    GraphModel *m_downstreamData;
    Settings m_settings;
    int m_updatePeriod;
    QTimer m_updateTimer;
    GraphModel *m_upstreamData;
    upnp::Service *m_wanCommonConfigService;
    QQuickView m_view;
};

} // namespace fritzmon

#endif // FRITZMON_MONITORAPP_HPP
