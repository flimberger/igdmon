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

#include "Settings.hpp"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTextCodec>

namespace fritzmon {

static constexpr auto *DEFAULT_HOST = "";
static constexpr auto DEFAULT_PORT = 0;
static constexpr auto DEFAULT_ENCRYPTION = false;
static constexpr auto *TEXT_ENCODING = "UTF-8";
static constexpr auto *CONNECTION_GROUP = "connection";
static constexpr auto *HOST_KEY = "host";
static constexpr auto *PORT_KEY = "port";
static constexpr auto *USE_SSL_KEY = "use_ssl";
static constexpr auto *HTTP_SCHEME = "http";
static constexpr auto *HTTPS_SCHEME = "https";

Settings::Settings(QObject *parent) : QObject(parent)
{}

void Settings::setHost(const QString &newHost)
{
    m_deviceURL.setAuthority(newHost);

    emit hostChanged(newHost);
}

QString Settings::host() const
{
    return m_deviceURL.host();
}

void Settings::setPort(int newPort)
{
    m_deviceURL.setPort(newPort);

    emit portChanged(newPort);
}

int Settings::port() const
{
    return m_deviceURL.port();
}

void Settings::setUseSSL(bool newUseSSL)
{
    setEncryption(newUseSSL);

    emit useSSLChanged(newUseSSL);
}

bool Settings::useSSL() const
{
    return m_deviceURL.scheme() == HTTPS_SCHEME;
}

QUrl Settings::deviceURL() const
{
    return m_deviceURL;
}

void Settings::readConfiguration()
{
    QSettings settings;

    settings.setDefaultFormat(QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName(TEXT_ENCODING));

    settings.beginGroup(CONNECTION_GROUP);
    m_deviceURL.setHost(settings.value(HOST_KEY, DEFAULT_HOST).toString());
    m_deviceURL.setPort(settings.value(PORT_KEY, DEFAULT_PORT).toInt());
    setEncryption(settings.value(USE_SSL_KEY, DEFAULT_ENCRYPTION).toBool());
    settings.endGroup();
}

void Settings::writeConfiguration()
{
    QSettings settings;

    settings.setDefaultFormat(QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName(TEXT_ENCODING));

    settings.beginGroup(CONNECTION_GROUP);
    settings.setValue(HOST_KEY, m_deviceURL.host());
    settings.setValue(PORT_KEY, m_deviceURL.port());
    settings.setValue(USE_SSL_KEY, m_deviceURL.scheme() == HTTPS_SCHEME);
}

void Settings::setEncryption(bool useSSL)
{
    if (useSSL)
        m_deviceURL.setScheme(HTTPS_SCHEME);
    else
        m_deviceURL.setScheme(HTTP_SCHEME);
}

} // namespace fritzmon
