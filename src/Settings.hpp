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
