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

#ifndef UPNP_DEVICE_HPP
#define UPNP_DEVICE_HPP

#include <QtCore/QString>
#include <QtCore/QUrl>

#include <memory>
#include <vector>

namespace fritzmon {
namespace upnp {

class Service;

namespace internal {

class DeviceBuilder;

} // namespace internal

class Device
{
public:
    ~Device();

    const std::vector<std::unique_ptr<Device>> &children() const;
    QString description() const;
    QString friendlyName() const;
    bool hasChildren() const;
    bool isRootDevice() const;
    QString manufacturerName() const;
    QUrl manufacturerURL() const;
    QString modelName() const;
    QString modelNumber() const;
    QUrl modelURL() const;
    Device *parentDevice() const;
    QUrl presentationURL() const;
    Device *rootDevice() const;
    QString serialNumber() const;
    const std::vector<std::unique_ptr<Service>> &services() const;
    QString type() const;
    QString uniqueDeviceName() const;
    QString upc() const;
    QUrl iconURL() const;

private:
    // clients use the builder to create instances
    Device();

    std::vector<std::unique_ptr<Device>> m_children;
    std::vector<std::unique_ptr<Service>> m_services;
    QString m_description;
    QString m_friendlyName;
    QString m_manufacturerName;
    QUrl m_manufacturerURL;
    QString m_modelName;
    QString m_modelNumber;
    QUrl m_modelURL;
    Device *m_parentDevice;
    QUrl m_presentationURL;
    Device *m_rootDevice;
    QString m_serialNumber;
    QString m_type;
    QString m_uniqueDeviceName;
    QString m_upc;
    QUrl m_iconURL;

    friend class internal::DeviceBuilder;
};

} // namespace upnp
} // namespace fritzmon

#endif // UPNP_DEVICE_HPP
