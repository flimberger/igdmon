#ifndef UPNP_DEVICEFINDER_HPP
#define UPNP_DEVICEFINDER_HPP

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkAccessManager>

#include <memory>
#include <vector>

class QNetworkReply;

namespace fritzmon {
namespace upnp {

class Device;

namespace internal {

class DeviceBuilder;

} // namespace internal

class DeviceFinder : public QObject
{
    Q_OBJECT

public:
    DeviceFinder(QObject *parent=nullptr);
    ~DeviceFinder();

    /* Strict UPNP would provide the following method::
     *
     *     void startFind();
     *
     * This method is not provided in this implementation, since the FritzBox implementation of
     * TR-064 does not advertise its UPNP capabilities.  Instead, the method ``findDevice`` is
     * provided, which takes the device URL as parameter.
     */
    void cancelFind();
    void findDevice(const QUrl &descriptionDocumentURL);

    const std::vector<std::unique_ptr<Device> > &devices() const;
    bool searching() const;

Q_SIGNALS:
    void deviceAdded(Device *device);
    void deviceRemoved(const QString &udn);
    void searchComplete();

private:
    Q_SLOT void deviceDescriptionReceived(QNetworkReply *reply);
    Q_SLOT void deviceDetected(internal::DeviceBuilder *deviceBuilder);
    void parseDeviceDescription(const QByteArray &data);

    QNetworkAccessManager m_networkAccess;
    std::vector<std::unique_ptr<internal::DeviceBuilder>> m_deviceBuilders;
    std::vector<std::unique_ptr<Device>> m_devices;
    QUrl m_baseURL;
    bool m_searching;

    Q_DISABLE_COPY(DeviceFinder)
};

} // namespace upnp
} // namespace fritzmon

#endif // UPNP_DEVICEFINDER_HPP
