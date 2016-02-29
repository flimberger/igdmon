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
