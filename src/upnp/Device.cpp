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
