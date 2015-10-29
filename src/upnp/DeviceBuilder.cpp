#include "DeviceBuilder.hpp"

#include "util.hpp"

#include "Device.hpp"
#include "Service.hpp"
#include "ServiceBuilder.hpp"

#include <algorithm>

namespace fritzmon {
namespace upnp {
namespace internal {

DeviceBuilder::DeviceBuilder(QObject *parent)
  : QObject(parent),
    m_instance(new Device())
{}

DeviceBuilder::~DeviceBuilder() = default;

DeviceBuilder &DeviceBuilder::addChild(std::unique_ptr<DeviceBuilder> child)
{
    m_subDeviceBuilders.emplace_back(std::move(child));

    auto result = connect(m_subDeviceBuilders.back().get(), &DeviceBuilder::finished,
                          this,                            &DeviceBuilder::onDeviceBuilderFinished);
    Q_ASSERT(result);
    Q_UNUSED(result);

    return *this;
}

DeviceBuilder &DeviceBuilder::addService(std::unique_ptr<ServiceBuilder> builder)
{
    m_serviceBuilders.emplace_back(std::move(builder));

    auto result = connect(m_serviceBuilders.back().get(), &ServiceBuilder::finished,
                          this,                           &DeviceBuilder::onServiceDetected);
    Q_UNUSED(result);
    m_serviceBuilders.back()->startDetection();

    return *this;
}

DeviceBuilder &DeviceBuilder::description(const QString &description)
{
    m_instance->m_description = description;
    return *this;
}

DeviceBuilder &DeviceBuilder::friendlyName(const QString &name)
{
    m_instance->m_friendlyName = name;
    return *this;
}

DeviceBuilder &DeviceBuilder::manufacturerName(const QString &manufacturerName)
{
    m_instance->m_manufacturerName = manufacturerName;
    return *this;
}

DeviceBuilder &DeviceBuilder::manufacturerURL(const QString &manufacturerURL)
{
    m_instance->m_manufacturerURL = manufacturerURL;
    return *this;
}

DeviceBuilder &DeviceBuilder::modelName(const QString &modelName)
{
    m_instance->m_modelName = modelName;
    return *this;
}

DeviceBuilder &DeviceBuilder::modelNumber(const QString &modelNumber)
{
    m_instance->m_modelNumber = modelNumber;
    return *this;
}

DeviceBuilder &DeviceBuilder::modelURL(const QString &modelURL)
{
    m_instance->m_modelURL = modelURL;
    return *this;
}

DeviceBuilder &DeviceBuilder::parentDevice(Device *parentDevice)
{
    m_instance->m_parentDevice = parentDevice;
    return *this;
}

DeviceBuilder &DeviceBuilder::presentationURL(const QString presentationURL)
{
    m_instance->m_presentationURL = presentationURL;
    return *this;
}

DeviceBuilder &DeviceBuilder::rootDevice(Device *rootDevice)
{
    m_instance->m_rootDevice = rootDevice;
    return *this;
}

DeviceBuilder &DeviceBuilder::serialNumber(const QString &serialNumber)
{
    m_instance->m_serialNumber = serialNumber;
    return *this;
}

DeviceBuilder &DeviceBuilder::type(const QString &type)
{
    m_instance->m_type = type;
    return *this;
}

DeviceBuilder &DeviceBuilder::uniqueDeviceName(const QString &udn)
{
    m_instance->m_uniqueDeviceName = udn;
    return *this;
}

DeviceBuilder &DeviceBuilder::upc(const QString &upc)
{
    m_instance->m_upc = upc;
    return *this;
}

DeviceBuilder &DeviceBuilder::iconURL(const QString &url)
{
    m_instance->m_iconURL = url;
    return *this;
}

std::unique_ptr<Device> DeviceBuilder::create()
{
    auto ptr = std::unique_ptr<Device>();

    m_instance.swap(ptr);

    return ptr;
}

void DeviceBuilder::onServiceDetected()
{
    auto *sender = qobject_cast<ServiceBuilder *>(QObject::sender());
    auto serviceBuilder = removeSmartpointerFromVector(m_serviceBuilders, sender);

    if (serviceBuilder)
        m_instance->m_services.push_back(serviceBuilder->create());
    else
        qDebug() << "DeviceBuilder::onServiceDetected: service not in builder pool";
    checkFinished();
}

void DeviceBuilder::onDeviceBuilderFinished()
{
    auto *sender = qobject_cast<DeviceBuilder *>(QObject::sender());
    auto builder = removeSmartpointerFromVector(m_subDeviceBuilders, sender);

    if (builder)
        m_instance->m_children.push_back(builder->create());
    else
        qDebug() << "DeviceBuilder::onDeviceBuilderFinished: device not in builder pool";
    checkFinished();
}

void DeviceBuilder::checkFinished()
{
    if ((m_subDeviceBuilders.size() == 0) && (m_serviceBuilders.size() == 0))
        emit finished();
}

} // namespace internal
} // namespace upnp
} // namespace fritzmon
