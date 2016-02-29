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
