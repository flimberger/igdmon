#include "Service.hpp"

#include "StateVariable.hpp"

#include "soap/IMessageBodyHandler.hpp"
#include "soap/Request.hpp"

#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

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
    bool endElement(const QString &tag) override;

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
};

GetAddonInfoResponseHandler::GetAddonInfoResponseHandler(const QString &namespaceURI,
                                                         const std::function<void(const QVariantMap&, const QVariant &)> &finishedCallback)
  : soap::IMessageBodyHandler(namespaceURI),
    m_finishedCallback(finishedCallback),
    m_state(ParserState::Root)
{}

bool GetAddonInfoResponseHandler::startElement(const QString &tag, QXmlStreamReader &stream)
{
    if ((m_state == ParserState::Root) && (tag == GET_ADDON_INFOS_RESPONSE_TAG))
        m_state = ParserState::Envelope;
    else
        m_outputArguments[tag] = stream.readElementText();

    return true;
}

bool GetAddonInfoResponseHandler::endElement(const QString &tag)
{
    if ((m_state == ParserState::Envelope) && (tag == GET_ADDON_INFOS_RESPONSE_TAG))
        m_finishedCallback(m_outputArguments, QVariant());

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
    auto *handler = new GetAddonInfoResponseHandler(WAN_COMMON_INTERFACE_CONFIG_NAMESPACE_URI,
                                                std::bind(&Service::onActionFinished, this,
                                                          std::placeholders::_1,
                                                          std::placeholders::_2));
    auto handlerPtr = std::shared_ptr<GetAddonInfoResponseHandler>(handler);

    m_request->addMessageHandler(WAN_COMMON_INTERFACE_CONFIG_NAMESPACE_URI, handlerPtr);
    m_request->addMessageHandler("", handlerPtr);
}

Service::~Service() = default;

void Service::invokeAction(const QString &name, const QVariantMap &inputArguments)
{
    Q_UNUSED(inputArguments);

    auto soapAction = m_type + "#" + name;
    auto soapBody = QString("<u:%1 xmlns:u=\"%2\"></u:%1>").arg(name).arg(m_type);

    m_request->start(m_controlURL, soapAction, soapBody);
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
//    auto soapBody = QString("<u:%1 xmlns:u=\"%2\"><u:varName>%3</u:varName></u:%1>")
//            .arg(QUERY_STATE_VARIABLE).arg(UPNP_CONTROL_NAMESPACE_URI).arg(name);

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
    emit actionInvoked(outputArguments, returnValue);
}

} // namespace upnp
} // namespace fritzmon
