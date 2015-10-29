#include "Request.hpp"

#include <QtCore/QDebug>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace fritzmon {
namespace soap {

static constexpr auto *SOAPACTION_HEADER = "SOAPACTION";
static constexpr auto *CONTENT_TYPE = "text/xml; charset=\"utf-8\"";
static constexpr auto *ENVELOPE_BEGIN = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\"><s:Body>";
static constexpr auto *ENVELOPE_END = "</s:Body></s:Envelope>";

Request::Request(QObject *parent)
  : QObject(parent),
    m_networkAccess()
{
    connect(&m_networkAccess, &QNetworkAccessManager::finished, this, &Request::onRequestCompleted);
}

void Request::start(const QUrl &url, const QString &action, const QString &bodyText)
{
    auto requestText = QString(ENVELOPE_BEGIN).append(bodyText).append(ENVELOPE_END).toUtf8();
    auto request = QNetworkRequest(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, CONTENT_TYPE);
    request.setRawHeader(SOAPACTION_HEADER, action.toUtf8());
    m_networkAccess.post(request, requestText);
}

void Request::onRequestCompleted(QNetworkReply *reply)
{
    auto rawText = std::make_shared<QByteArray>(reply->readAll());

    emit finished(rawText);
}

} // namespace soap
} // namespace fritzmon
