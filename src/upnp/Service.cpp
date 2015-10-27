#include "Service.hpp"

namespace fritzmon {
namespace upnp {

Service::Service(QObject *parent)
  : QObject(parent),
    m_actions(),
    m_type(),
    m_id(),
    m_scpdURL(),
    m_controlURL(),
    m_eventSubURL()
{}

QString Service::id() const
{
    return m_id;
}

int Service::lastTransportStatus() const
{
    return 404;
}

QString Service::serviceTypeIdentifier() const
{
    return m_type;
}

const QVector<Action> &Service::actions() const
{
    return m_actions;
}

} // namespace upnp
} // namespace fritzmon
