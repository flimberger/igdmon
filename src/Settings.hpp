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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

namespace fritzmon {

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(bool useSSL READ useSSL WRITE setUseSSL NOTIFY useSSLChanged)

public:
    explicit Settings(QObject *parent = nullptr);

    void setHost(const QString &newHost);
    QString host() const;

    void setPort(int newPort);
    int port() const;

    void setUseSSL(bool newUseSSL);
    bool useSSL() const;

    QUrl deviceURL() const;

    // doesn't emit the changed signals, because all three would be emitted shortly after each
    // other, and the changes are expected by the caller
    void readConfiguration();
    void writeConfiguration();

signals:
    void hostChanged(QString newHost);
    void portChanged(int newPort);
    void useSSLChanged(bool newUseSSL);

private:
    void setEncryption(bool useSSL);

    QUrl m_deviceURL;

    Q_DISABLE_COPY(Settings)
};

} // namespace fritzmon

#endif // SETTINGS_HPP
