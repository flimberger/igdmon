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

#include "upnp/Device.hpp"

#include "upnp/Service.hpp"

namespace fritzmon {
namespace upnp {

Device::Device()
  : m_children(),
    m_services(),
    m_description(),
    m_friendlyName(),
    m_manufacturerName(),
    m_manufacturerURL(),
    m_modelName(),
    m_modelNumber(),
    m_modelURL(),
    m_parentDevice(nullptr),
    m_presentationURL(),
    m_rootDevice(nullptr),
    m_serialNumber(),
    m_type(),
    m_uniqueDeviceName(),
    m_upc(),
    m_iconURL()
{}

Device::~Device() = default;

const std::vector<std::unique_ptr<Device> > &Device::children() const
{
    return m_children;
}

QString Device::description() const
{
    return m_description;
}

QString Device::friendlyName() const
{
    return m_friendlyName;
}

bool Device::hasChildren() const
{
    return m_children.size() != 0;
}

bool Device::isRootDevice() const
{
    return m_parentDevice == nullptr;
}

QString Device::manufacturerName() const
{
    return m_manufacturerName;
}

QUrl Device::manufacturerURL() const
{
    return m_manufacturerURL;
}

QString Device::modelName() const
{
    return m_modelName;
}

QString Device::modelNumber() const
{
    return m_modelNumber;
}

QUrl Device::modelURL() const
{
    return m_modelURL;
}

Device *Device::parentDevice() const
{
    return m_parentDevice;
}

QUrl Device::presentationURL() const
{
    return m_presentationURL;
}

Device *Device::rootDevice() const
{
    return m_rootDevice;
}

QString Device::serialNumber() const
{
    return m_serialNumber;
}

const std::vector<std::unique_ptr<Service> > &Device::services() const
{
    return m_services;
}

QString Device::type() const
{
    return m_type;
}

QString Device::uniqueDeviceName() const
{
    return m_uniqueDeviceName;
}

QString Device::upc() const
{
    return m_upc;
}

QUrl Device::iconURL() const
{
    return m_iconURL;
}

} // namespace upnp
} // namespace fritzmon
