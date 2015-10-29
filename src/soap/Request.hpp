#ifndef FRITZMON_SOAP_REQUEST_HPP
#define FRITZMON_SOAP_REQUEST_HPP

#include <QtCore/QObject>

#include <QtNetwork/QNetworkAccessManager>

#include <memory>

class QNetworkReply;

namespace fritzmon {
namespace soap {

class Request : public QObject
{
    Q_OBJECT

public:
    explicit Request(QObject *parent=nullptr);

    void start(const QUrl &url, const QString &action, const QString &bodyText);

Q_SIGNALS:
    void finished(std::shared_ptr<QByteArray> rawText);

private:
    Q_SLOT void onRequestCompleted(QNetworkReply *reply);

    QNetworkAccessManager m_networkAccess;

    Q_DISABLE_COPY(Request)
};

} // namespace soap
} // namespace fritzmon

#endif // FRITZMON_SOAP_REQUEST_HPP
