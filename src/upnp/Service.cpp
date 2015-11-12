#include "Service.hpp"

#include "StateVariable.hpp"

#include "soap/IMessageBodyHandler.hpp"
#include "soap/Request.hpp"

#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

namespace fritzmon {
namespace upnp {

static constexpr auto *UPNP_CONTROL_NAMESPACE_URI = "urn:schemas-upnp-org:control-1-0";
static constexpr auto *QUERY_STATE_VARIABLE = "QueryStateVariable";
static constexpr auto *VAR_NAME = "varName";
static constexpr auto *GET_RECV_BYTES_NS_URI = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";

class GetTotalBytesReceivedResponseHandler : public soap::IMessageBodyHandler
{
public:
    GetTotalBytesReceivedResponseHandler(const QString &namespaceURI);

    bool startElementHandler(const QString &tag, QXmlStreamReader &stream) override;
    bool endElementHandler(const QString &tag) override;

    int value() const;

private:
    static constexpr auto *GET_RECV_BYTES_RESPONSE_TAG = "GetTotalBytesReceivedResponse";
    static constexpr auto *GET_RECV_BYTES_TAG = "NewTotalBytesSent";

    enum class ParserState {
        Root,
        Envelope,
    } m_state;
    int m_value;
};

GetTotalBytesReceivedResponseHandler::GetTotalBytesReceivedResponseHandler(const QString &namespaceURI)
  : soap::IMessageBodyHandler(namespaceURI),
    m_state(ParserState::Root),
    m_value(0)
{}

bool GetTotalBytesReceivedResponseHandler::startElementHandler(const QString &tag,
                                                               QXmlStreamReader &stream)
{
    if (m_state == ParserState::Root) {
        if (tag == GET_RECV_BYTES_RESPONSE_TAG)
            m_state = ParserState::Envelope;
    } else {
        if (tag == GET_RECV_BYTES_TAG) {
            m_value = stream.readElementText().toInt();
            qDebug() << "GetTotalBytesReceivedResponseHandler::startElementHandler: " << m_value;
        }
    }

    return true;
}

bool GetTotalBytesReceivedResponseHandler::endElementHandler(const QString &tag)
{
    if (m_state == ParserState::Envelope)
        if (tag == GET_RECV_BYTES_TAG)
            m_state = ParserState::Root;

    return true;
}

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
    m_request->addMessageHandler(GET_RECV_BYTES_NS_URI,
                                 std::make_shared<GetTotalBytesReceivedResponseHandler>(GET_RECV_BYTES_NS_URI));
    m_request->addMessageHandler("",
                                 std::make_shared<GetTotalBytesReceivedResponseHandler>(GET_RECV_BYTES_NS_URI));
}

Service::~Service() = default;

void Service::invokeAction(const QString &name, const QVariant *inputArguments,
                           QVariant *outputArguments, QVariant *returnValue)
{
    Q_UNUSED(inputArguments);
    Q_UNUSED(outputArguments);
    Q_UNUSED(returnValue);

    auto soapAction = m_type + "#" + name;
    auto soapBody = QString("<u:%1 xmlns:u=\"%2\"></u:%1>").arg(name).arg(m_type);

    m_request->start(m_controlURL, soapAction, soapBody);
}

void Service::queryStateVariable(const QString &name, QVariant &value)
{
    Q_UNUSED(value);

    auto soapAction = QString(UPNP_CONTROL_NAMESPACE_URI) + "#" + QUERY_STATE_VARIABLE;
    auto data = QByteArray();
    QXmlStreamWriter stream(&data);

    stream.writeStartElement(UPNP_CONTROL_NAMESPACE_URI, QUERY_STATE_VARIABLE);
    stream.writeStartElement(UPNP_CONTROL_NAMESPACE_URI, VAR_NAME);
    stream.writeCharacters(name);
    stream.writeEndElement();
    stream.writeEndElement();
//    auto soapBody = QString("<u:%1 xmlns:u=\"%2\"><u:varName>%3</u:varName></u:%1>")
//            .arg(QUERY_STATE_VARIABLE).arg(UPNP_CONTROL_NAMESPACE_URI).arg(name);

    qDebug() << "Service::queryStateVariable: " << data;
    m_request->start(m_controlURL, soapAction, data);
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

const std::vector<Action> &Service::actions() const
{
    return m_actions;
}

const std::vector<StateVariable> &Service::stateVariables() const
{
    return m_stateVariables;
}

} // namespace upnp
} // namespace fritzmon
