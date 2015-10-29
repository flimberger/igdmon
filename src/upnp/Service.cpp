#include "Service.hpp"

#include "soap/Request.hpp"

namespace fritzmon {
namespace upnp {

Service::Service(QObject *parent)
  : QObject(parent),
    m_actions(),
    m_type(),
    m_id(),
    m_scpdURL(),
    m_controlURL(),
    m_eventSubURL(),
    m_request(std::make_unique<soap::Request>())
{
    connect(m_request.get(), &soap::Request::finished, this, &Service::onRequestCompleted);
}

Service::~Service() = default;

void Service::invokeAction(const QString &name, const QVariant *inputArguments,
                           QVariant *outputArguments, QVariant *returnValue)
{
    Q_UNUSED(inputArguments);
    Q_UNUSED(outputArguments);
    Q_UNUSED(returnValue);

    auto soapAction = m_type + "#" + name;
    auto soapBody = QString("<u:%1 xmlns:u=\"%1\"></u:%1>").arg(name).arg(m_type);

    m_request->start(m_controlURL, soapAction, soapBody);
}

void Service::queryStateVariable(const QString &name, QVariant &value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

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

void Service::onRequestCompleted(std::shared_ptr<QByteArray> rawText)
{
    qDebug() << "Service::onRequestCompleted:" << *rawText.get();
}

} // namespace upnp
} // namespace fritzmon
