/*
 * Copyright (c) 2016 Florian Limberger <flo@snakeoilproductions.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Request.hpp"
#include "IMessageBodyHandler.hpp"

#include <QtCore/QDebug>
#include <QtCore/QXmlStreamReader>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSslError>

#include <tuple>

namespace fritzmon {
namespace soap {

#if defined(SOAP_REQUEST_DEBUG_DUMP)
static constexpr auto REQUEST_DEBUG_DUMP = true;
#else
static constexpr auto REQUEST_DEBUG_DUMP = false;
#endif

// static constexpr auto *SOAP_NAMESPACE_URI = "http://www.w3.org/2003/05/soap-envelope";
static constexpr auto *SOAP_NAMESPACE_URI = "http://schemas.xmlsoap.org/soap/envelope/";
static constexpr auto *SOAP_ROOT = "Envelope";
static constexpr auto *SOAP_HEADER = "Header";
static constexpr auto *SOAP_BODY = "Body";

static constexpr auto *SOAPACTION_HEADER = "SOAPACTION";
static constexpr auto *CONTENT_TYPE = "text/xml; charset=\"utf-8\"";
static constexpr auto *ENVELOPE_BEGIN = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\"><s:Body>";
static constexpr auto *ENVELOPE_END = "</s:Body></s:Envelope>";

Request::Request(QObject *parent)
  : QObject(parent),
    m_networkAccess(),
    m_messageBodyHandlers()
{
    connect(&m_networkAccess, &QNetworkAccessManager::finished, this, &Request::onRequestCompleted);
    connect(&m_networkAccess, &QNetworkAccessManager::sslErrors, this, &Request::onSslErrors);
}

Request::~Request() = default;

void Request::addMessageHandler(const QString &namespaceURI,
                                const std::shared_ptr<IMessageBodyHandler> &handler)
{
    m_messageBodyHandlers.emplace_back(namespaceURI, handler);
}

void Request::start(const QUrl &url, const QString &action, const QString &bodyText)
{
    auto requestText = QString(ENVELOPE_BEGIN).append(bodyText).append(ENVELOPE_END).toUtf8();
    auto request = QNetworkRequest(url);

    if (REQUEST_DEBUG_DUMP) {
        qDebug() << "Request::start: >>>>>>>>";
        qDebug() << "Request::start: " << requestText;
        qDebug() << "Request::start: >>>>>>>>";
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, CONTENT_TYPE);
    request.setRawHeader(SOAPACTION_HEADER, action.toUtf8());
    m_networkAccess.post(request, requestText);
}

void Request::onRequestCompleted(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
        qDebug() << "Request::onRequestCompleted:" << reply->errorString();
    else
        parseReply(reply->readAll());

    emit finished();
}

void Request::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    if ((errors.size() == 1) && (errors[0].error() == QSslError::SelfSignedCertificate)) {
        qDebug() << "Request::onSslErrors: self-signed certificate (ignored)";
        reply->ignoreSslErrors();
    }
}

void Request::parseReply(const QByteArray &data)
{
    QXmlStreamReader stream(data);
    enum class ParserState {
        Start,
        Header,
        Body
    } state = ParserState::Start;

    if (REQUEST_DEBUG_DUMP) {
        qDebug() << "Request::parseReply: <<<<<<<<";
        qDebug() << "Request::parseReply:" << data;
        qDebug() << "Request::parseReply: <<<<<<<<";
    }
    if (stream.atEnd()) {
        qDebug() << "Request::parseReply: empty description document";

        return;
    }
    for (const auto &handler : m_messageBodyHandlers)
        std::get<1>(handler)->startMessage();
        // TODO: how to react on errors? For now, they are simply ignored

    // start reading
    stream.readNext();
    // read root element and "verify" schema
    stream.readNext();
    if ((stream.namespaceUri() != SOAP_NAMESPACE_URI) || (stream.name() != SOAP_ROOT)) {
        qDebug() << "Request::parseReply: parse error: no SOAP envelope";
        qDebug() << "Request::parseReply: expected" << SOAP_NAMESPACE_URI << ":"
                 << SOAP_ROOT;
        qDebug() << "Request::parseReply: received" << stream.namespaceUri() << ":"
                 << stream.name();

        return;
    }
    for (stream.readNext(); !stream.atEnd(); stream.readNext()) {
        auto token = stream.tokenType();
        auto tag = stream.name();

        switch (state) {
        case ParserState::Start:
            if (token == QXmlStreamReader::StartElement) {
                if (tag == SOAP_HEADER)
                    state = ParserState::Header;
                else if (tag == SOAP_BODY)
                    state = ParserState::Body;
            }
            break;
        case ParserState::Header:
            // ignore the header for now
            if ((token == QXmlStreamReader::EndElement) && (tag == SOAP_HEADER))
                state = ParserState::Start;
            break;
        case ParserState::Body:
            auto namespaceURI = stream.namespaceUri();

            if ((token == QXmlStreamReader::EndElement) && (tag == SOAP_BODY))
                state = ParserState::Start;
            else if ((token == QXmlStreamReader::EndElement)
                     || (token == QXmlStreamReader::StartElement))
                for (auto &handlerPair : m_messageBodyHandlers)
                    if (std::get<0>(handlerPair) == namespaceURI) {
                        if (token == QXmlStreamReader::StartElement)
                            std::get<1>(handlerPair)->startElement(tag.toString(), stream);
                        else
                            std::get<1>(handlerPair)->endElement(tag.toString());
                    }
            break;
        }
    }
    if (stream.hasError())
        qDebug() << "Request::parseReply: parse error:" << stream.errorString();
    for (const auto &handler : m_messageBodyHandlers)
        std::get<1>(handler)->endMessage();
}

} // namespace soap
} // namespace fritzmon
