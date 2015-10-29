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
