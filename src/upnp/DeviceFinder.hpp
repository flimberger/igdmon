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
    Q_SLOT void onDeviceFinished();
    Q_SLOT void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
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
