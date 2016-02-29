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

#ifndef DEVICEBUILDER_HPP
#define DEVICEBUILDER_HPP

#include <QtCore/QObject>

#include <memory>
#include <vector>

class QString;

namespace fritzmon {
namespace upnp {

class Device;

namespace internal {

class ServiceBuilder;

class DeviceBuilder : public QObject
{
    Q_OBJECT

public:
    explicit DeviceBuilder(QObject *parent=nullptr);
    ~DeviceBuilder();

    DeviceBuilder &addChild(std::unique_ptr<DeviceBuilder> addChild);
    // Calls ``builder->startDetection()``, so ``scpdURL`` *must* be set
    DeviceBuilder &addService(std::unique_ptr<ServiceBuilder> builder);
    DeviceBuilder &description(const QString &description);
    DeviceBuilder &friendlyName(const QString &name);
    DeviceBuilder &manufacturerName(const QString &manufacturerName);
    DeviceBuilder &manufacturerURL(const QString &manufacturerURL);
    DeviceBuilder &modelName(const QString &modelName);
    DeviceBuilder &modelNumber(const QString &modelNumber);
    DeviceBuilder &modelURL(const QString &modelURL);
    DeviceBuilder &parentDevice(Device *parentDevice);
    DeviceBuilder &presentationURL(const QString presentationURL);
    DeviceBuilder &rootDevice(Device *rootDevice);
    DeviceBuilder &serialNumber(const QString &serialNumber);
    DeviceBuilder &type(const QString &type);
    DeviceBuilder &uniqueDeviceName(const QString &udn);
    DeviceBuilder &upc(const QString &upc);
    DeviceBuilder &iconURL(const QString &url);

    std::unique_ptr<Device> create();

Q_SIGNALS:
    void finished();

private:
    Q_SLOT void onServiceDetected();
    Q_SLOT void onDeviceBuilderFinished();
    std::unique_ptr<ServiceBuilder> removeServiceBuilderFromPool(ServiceBuilder *serviceBuilder);
    void checkFinished();

    std::vector<std::unique_ptr<ServiceBuilder>> m_serviceBuilders;
    std::vector<std::unique_ptr<DeviceBuilder>> m_subDeviceBuilders;
    std::unique_ptr<Device> m_instance;

    Q_DISABLE_COPY(DeviceBuilder)
};

} // namespace internal
} // namespace upnp
} // namespace fritzmon

#endif // DEVICEBUILDER_HPP
