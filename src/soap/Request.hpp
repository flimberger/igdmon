#ifndef FRITZMON_SOAP_REQUEST_HPP
#define FRITZMON_SOAP_REQUEST_HPP

#include <QtCore/QList>
#include <QtCore/QObject>

#include <QtNetwork/QNetworkAccessManager>

#include <memory>
#include <utility>
#include <string>
#include <vector>

class QNetworkReply;
class QSslError;

namespace fritzmon {
namespace soap {

class IMessageBodyHandler;

class Request : public QObject
{
    Q_OBJECT

public:
    explicit Request(QObject *parent=nullptr);
    ~Request();

    void addMessageHandler(const QString &namespaceURI,
                           const std::shared_ptr<IMessageBodyHandler> &handler);
    void start(const QUrl &url, const QString &action, const QString &bodyText);

Q_SIGNALS:
    void finished();

private:
    Q_SLOT void onRequestCompleted(QNetworkReply *reply);
    Q_SLOT void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void parseReply(const QByteArray &data);

    QNetworkAccessManager m_networkAccess;
    std::vector<std::pair<QString, std::shared_ptr<IMessageBodyHandler>>> m_messageBodyHandlers;

    Q_DISABLE_COPY(Request)
};

} // namespace soap
} // namespace fritzmon

#endif // FRITZMON_SOAP_REQUEST_HPP