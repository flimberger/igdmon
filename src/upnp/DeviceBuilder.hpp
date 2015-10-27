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

    bool done() const;

Q_SIGNALS:
    void finished();

private:
    Q_SLOT void serviceDetected(ServiceBuilder *sender);
    Q_SLOT void onServiceDetectionFailed(ServiceBuilder *serviceBuilder);
    Q_SLOT void onDeviceBuilderFinished();
    std::unique_ptr<ServiceBuilder> removeServiceBuilderFromPool(ServiceBuilder *serviceBuilder);
    void checkFinished();

    std::vector<std::unique_ptr<ServiceBuilder>> m_serviceBuilders;
    std::vector<std::unique_ptr<DeviceBuilder>> m_subDeviceBuilders;
    std::unique_ptr<Device> m_instance;
    bool m_done;

    Q_DISABLE_COPY(DeviceBuilder)
};

} // namespace internal
} // namespace upnp
} // namespace fritzmon

#endif // DEVICEBUILDER_HPP
