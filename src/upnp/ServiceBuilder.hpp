#ifndef UPNP_INTERNAL_SERVICEBUILDER_HPP
#define UPNP_INTERNAL_SERVICEBUILDER_HPP

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>

#include <QtNetwork/QNetworkAccessManager>

#include <memory>

class QNetworkReply;
class QUrl;

namespace fritzmon {
namespace upnp {

class Service;

namespace internal {

class ServiceBuilder : public QObject
{
    Q_OBJECT

public:
    ServiceBuilder(QObject *parent=nullptr);
    ~ServiceBuilder();

    ServiceBuilder &type(const QString &serviceType);
    ServiceBuilder &id(const QString &serviceId);
    ServiceBuilder &scpdURL(const QUrl &scpdURL);
    ServiceBuilder &controlURL(const QUrl &controlURL);
    ServiceBuilder &eventSubURL(const QUrl &eventURL);

    void startDetection();
    std::unique_ptr<Service> create();

Q_SIGNALS:
    void finished();

private:
    Q_SLOT void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    Q_SLOT void serviceDescriptionReceived(QNetworkReply *reply);
    void parseServiceDescription(const QByteArray &data);

    QNetworkAccessManager m_networkAccess;
    std::unique_ptr<Service> m_instance;

    Q_DISABLE_COPY(ServiceBuilder)
};

} // namespace internal
} // namespace upnp
} // namespace fritzmon

#endif // UPNP_INTERNAL_SERVICEBUILDER_HPP
