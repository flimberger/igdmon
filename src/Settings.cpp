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
