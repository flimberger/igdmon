#include "DeviceBuilder.hpp"

#include "Device.hpp"
#include "Service.hpp"
#include "ServiceBuilder.hpp"

#include <algorithm>

namespace fritzmon {
namespace upnp {
namespace internal {

DeviceBuilder::DeviceBuilder(QObject *parent)
  : QObject(parent),
    m_instance(new Device()),
    m_done(false)
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

    auto result = connect(m_serviceBuilders.back().get(), &ServiceBuilder::serviceDetected,
                          this,                           &DeviceBuilder::serviceDetected);
    Q_ASSERT(result);
    result = connect(m_serviceBuilders.back().get(), &ServiceBuilder::serviceDetectionFailed,
                     this, &DeviceBuilder::onServiceDetectionFailed);
    Q_ASSERT(result);
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

void DeviceBuilder::serviceDetected(ServiceBuilder *sender)
{
    auto serviceBuilder = removeServiceBuilderFromPool(sender);

    m_instance->m_services.push_back(serviceBuilder->create());
    qDebug() << "DeviceBuilder::serviceDetected: service detected:" << m_instance->m_services.back()->id();

    checkFinished();
}

void DeviceBuilder::onServiceDetectionFailed(ServiceBuilder *serviceBuilder)
{
    auto builder = removeServiceBuilderFromPool(serviceBuilder);

    Q_UNUSED(builder);
}

void DeviceBuilder::onDeviceBuilderFinished()
{
    using std::begin;
    using std::end;

    auto *sender = qobject_cast<DeviceBuilder *>(QObject::sender());
    auto ptr = std::unique_ptr<DeviceBuilder>();
    const auto b = begin(m_subDeviceBuilders);
    const auto e = end(m_subDeviceBuilders);

    // Find device builder in pool
    auto ptrit = std::find_if(b, e, [&](const std::unique_ptr<DeviceBuilder> &bptr){
        return bptr.get() == sender;
    });

    if (ptrit == e) {
        qDebug() << "DeviceBuilder::onDeviceBuilderFinished: failed to locate sender in pool";
        goto end;
    }

    // Swap pointer from pool into local instance and remove the new nullptr from the pool
    ptrit->swap(ptr);
    std::remove_if(b, e, [](const std::unique_ptr<DeviceBuilder> &bptr){ return bool(bptr); });

end:
    checkFinished();
}

std::unique_ptr<ServiceBuilder>
DeviceBuilder::removeServiceBuilderFromPool(ServiceBuilder *serviceBuilder)
{
    using std::begin;
    using std::end;

    auto ptr = std::unique_ptr<ServiceBuilder>();
    const auto b = begin(m_serviceBuilders);
    const auto e = end(m_serviceBuilders);

    // Find service builder in the pool
    auto ptrref = std::find_if(b, e, [&](const std::unique_ptr<ServiceBuilder> &builder){
        return builder.get() == serviceBuilder;
    });

    if (ptrref == e) {
        qDebug() << "DeviceBuilder: failed to find service builder";

        return ptr;
    }

    // Swap service builder from pool into local instance
    ptrref->swap(ptr);
    std::remove_if(begin(m_serviceBuilders), end(m_serviceBuilders),
                   [&](const std::unique_ptr<ServiceBuilder> &builder) {
        return bool(builder);
    });

    return ptr;
}

void DeviceBuilder::checkFinished()
{
    if (!m_done && (m_subDeviceBuilders.size() == 0) && (m_serviceBuilders.size() == 0)) {
        m_done = true;
        emit finished();
    }
}

} // namespace internal
} // namespace upnp
} // namespace fritzmon
