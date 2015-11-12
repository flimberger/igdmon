#include "Service.hpp"

#include "StateVariable.hpp"

#include "soap/IMessageBodyHandler.hpp"
#include "soap/Request.hpp"

#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

#include <algorithm>
#include <functional>

namespace fritzmon {
namespace upnp {

static constexpr auto *UPNP_CONTROL_NAMESPACE_URI = "urn:schemas-upnp-org:control-1-0";
static constexpr auto *QUERY_STATE_VARIABLE_TAG = "QueryStateVariable";
static constexpr auto *VAR_NAME_TAG = "varName";
static constexpr auto *WAN_COMMON_INTERFACE_CONFIG_NAMESPACE_URI = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";

class GetAddonInfoResponseHandler : public soap::IMessageBodyHandler
{
public:
    GetAddonInfoResponseHandler(const QString &namespaceURI,
                                const std::function<void(const QVariantMap&, const QVariant &)> &finishedCallback);

    bool startElement(const QString &tag, QXmlStreamReader &stream) override;
    bool startMessage() override;
    bool endElement(const QString &tag) override;
    bool endMessage() override;

private:
    static constexpr auto *GET_ADDON_INFOS_RESPONSE_TAG = "GetAddonInfosResponse";
    static constexpr auto *NEW_BYTE_SEND_RATE_TAG = "NewByteSendRate";
    static constexpr auto *NEW_BYTE_RECEIVE_RATE_TAG = "NewByteReceiveRate";
    static constexpr auto *NEW_PACKET_SEND_RATE_TAG = "NewPacketSendRate";
    static constexpr auto *NEW_PACKET_RECEIVE_RATE_TAG = "NewPacketReceiveRate";
    static constexpr auto *NEW_TOTAL_BYTES_SENT_TAG = "NewTotalBytesSent";
    static constexpr auto *NEW_TOTAL_BYTES_RECEIVED_TAG = "NewTotalBytesReceived";
    static constexpr auto *NEW_AUTO_DISCONNECT_TIME_TAG = "NewAutoDisconnectTime";
    static constexpr auto *NEW_IDLE_DISCONNECT_TIME_TAG = "NewIdleDisconnectTime";
    static constexpr auto *NEW_DNS_SERVER_1_TAG = "NewDNSServer1";
    static constexpr auto *NEW_DNS_SERVER_2_TAG = "NewDNSServer2";
    static constexpr auto *NEW_VOID_DNS_SERVER_1_TAG = "NewVoipDNSServer1";
    static constexpr auto *NEW_VOID_DNS_SERVER_2_TAG = "NewVoipDNSServer2";
    static constexpr auto *NEW_UPNP_CONTROL_ENABLED = "NewUpnpControlEnabled";
    static constexpr auto *NEW_ROUTED_BRIDGED_MODE_BOTH = "NewRoutedBridgedModeBoth";

    std::function<void(const QVariantMap&, const QVariant &)> m_finishedCallback;
    enum class ParserState {
        Root,
        Envelope,
    } m_state;
    QVariantMap m_outputArguments;

    // used to suppress more than one finalizations, if the handler is installed multiple times
    bool m_finalized;
};

GetAddonInfoResponseHandler::GetAddonInfoResponseHandler(const QString &namespaceURI,
                                                         const std::function<void(const QVariantMap&, const QVariant &)> &finishedCallback)
  : soap::IMessageBodyHandler(namespaceURI),
    m_finishedCallback(finishedCallback)
{}

bool GetAddonInfoResponseHandler::startElement(const QString &tag, QXmlStreamReader &stream)
{
    if ((m_state == ParserState::Root) && (tag == GET_ADDON_INFOS_RESPONSE_TAG))
        m_state = ParserState::Envelope;
    else
        m_outputArguments[tag] = stream.readElementText();

    return true;
}

bool GetAddonInfoResponseHandler::startMessage()
{
    m_finalized = false;
    m_state = ParserState::Root;
    m_outputArguments.clear();

    return true;
}

bool GetAddonInfoResponseHandler::endElement(const QString &tag)
{
    if ((m_state == ParserState::Envelope) && (tag == GET_ADDON_INFOS_RESPONSE_TAG))
        m_state = ParserState::Root;

    return true;
}

bool GetAddonInfoResponseHandler::endMessage()
{
    if (!m_finalized) {
        m_finishedCallback(m_outputArguments, QVariant());
        m_finalized = true;
    }
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
    m_request(std::make_unique<soap::Request>()),
    m_invokationPending(false)
{
    auto handler = std::make_shared<GetAddonInfoResponseHandler>(
                WAN_COMMON_INTERFACE_CONFIG_NAMESPACE_URI, std::bind(&Service::onActionFinished,
                                                                     this, std::placeholders::_1,
                                                                     std::placeholders::_2));

    m_request->addMessageHandler(WAN_COMMON_INTERFACE_CONFIG_NAMESPACE_URI, handler);
    m_request->addMessageHandler("", handler);
}

Service::~Service() = default;

Service::InvokeActionResult Service::invokeAction(const QString &name, const QVariantMap &inputArguments)
{
    if (m_invokationPending)
        return InvokeActionResult::PendingAction;

    if (!std::any_of(std::cbegin(m_actions), std::cend(m_actions), [&name](const Action &action) {
            return action.name() == name;
        }))
        return InvokeActionResult::InvalidAction;

    m_invokationPending = true;

    const auto end = std::cend(inputArguments);
    auto soapAction = m_type + "#" + name;
    auto soapBody = QByteArray();
    QXmlStreamWriter stream(&soapBody);

    stream.writeStartElement(m_type, name);
    if (!inputArguments.empty())
        for (auto i = std::cbegin(inputArguments); i != end; ++i)
            stream.writeTextElement(QString(), i.key(), i.value().toString());
    stream.writeEndElement();

    m_request->start(m_controlURL, soapAction, soapBody);

    return InvokeActionResult::Success;
}

void Service::queryStateVariable(const QString &name, QVariant &value)
{
    Q_UNUSED(value);

    auto soapAction = QString(UPNP_CONTROL_NAMESPACE_URI) + "#" + QUERY_STATE_VARIABLE_TAG;
    auto data = QByteArray();
    QXmlStreamWriter stream(&data);

    stream.writeStartElement(UPNP_CONTROL_NAMESPACE_URI, QUERY_STATE_VARIABLE_TAG);
    stream.writeStartElement(UPNP_CONTROL_NAMESPACE_URI, VAR_NAME_TAG);
    stream.writeCharacters(name);
    stream.writeEndElement();
    stream.writeEndElement();

    qDebug() << "Service::queryStateVariable: " << data;
    m_request->start(m_controlURL, soapAction, data);
}

QString Service::id() const
{
    return m_id;
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

void Service::onActionFinished(const QVariantMap &outputArguments, const QVariant &returnValue)
{
    m_invokationPending = false;

    emit actionInvoked(outputArguments, returnValue);
}

} // namespace upnp
} // namespace fritzmon
