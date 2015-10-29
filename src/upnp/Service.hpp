#ifndef UPNP_SERVICE_HPP
#define UPNP_SERVICE_HPP

#include <upnp/Action.hpp>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVector>

class QVariant;

namespace fritzmon {
namespace upnp {

namespace internal {

class ServiceBuilder;

} // namespace internal

class Service : public QObject
{
    Q_OBJECT

public:
    explicit Service(QObject *parent=nullptr);

    void invoceAction(const QString &name, const QVariant *inputArguments,
                      QVariant *outputArguments, QVariant *returnValue);
    void queryStateVariable(const QString &name, QVariant &value);

    QString id() const;
    int lastTransportStatus() const;
    QString serviceTypeIdentifier() const;

    const QVector<Action> &actions() const;

Q_SIGNALS:
    void serviceInstanceDied();
    void stateVariableChanged(const QString &name, const QVariant &value);

private:
    QVector<Action> m_actions;
    QString m_type;
    QString m_id;
    QUrl m_scpdURL;
    QUrl m_controlURL;
    QUrl m_eventSubURL;

    friend class internal::ServiceBuilder;
    Q_DISABLE_COPY(Service)
};

} // namespace upnp
} // namespace fritzmon

#endif // UPNP_SERVICE_HPP
