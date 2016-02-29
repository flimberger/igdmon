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

using MessageFinishedCallback = std::function<void(const QVariantMap&, const QVariant &)>;

static constexpr auto *UPNP_CONTROL_NAMESPACE_URI = "urn:schemas-upnp-org:control-1-0";
static constexpr auto *QUERY_STATE_VARIABLE_TAG = "QueryStateVariable";
static constexpr auto *RESPONSE_SUFFIX = "Response";
static constexpr auto *VAR_NAME_TAG = "varName";
static constexpr auto *WAN_COMMON_INTERFACE_CONFIG_NAMESPACE_URI = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";

class UpnpResponseHandler : public soap::IMessageBodyHandler
{
public:
    UpnpResponseHandler(const QString &namespaceURI,
                                const MessageFinishedCallback &finishedCallback);

    bool startElement(const QString &tag, QXmlStreamReader &stream) override;
    bool startMessage() override;
    bool endElement(const QString &tag) override;
    bool endMessage() override;

private:
    MessageFinishedCallback m_finishedCallback;
    QVariantMap m_outputArguments;
    enum class ParserState {
        Root,
        Envelope,
    } m_parserState;

    // used to suppress more than one finalizations, if the handler is installed multiple times
    bool m_finalized;
};

UpnpResponseHandler::UpnpResponseHandler(const QString &namespaceURI,
                                         const MessageFinishedCallback &finishedCallback)
  : soap::IMessageBodyHandler(namespaceURI),
    m_finishedCallback(finishedCallback)
{}

bool UpnpResponseHandler::startElement(const QString &tag, QXmlStreamReader &stream)
{
    if ((m_parserState == ParserState::Root) && tag.endsWith(RESPONSE_SUFFIX))
        m_parserState = ParserState::Envelope;
    else
        m_outputArguments[tag] = stream.readElementText();

    return true;
}

bool UpnpResponseHandler::startMessage()
{
    m_finalized = false;
    m_parserState = ParserState::Root;
    m_outputArguments.clear();

    return true;
}

bool UpnpResponseHandler::endElement(const QString &tag)
{
    if ((m_parserState == ParserState::Envelope) && tag.endsWith(RESPONSE_SUFFIX))
        m_parserState = ParserState::Root;

    return true;
}

bool UpnpResponseHandler::endMessage()
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
    auto handler = std::make_shared<UpnpResponseHandler>(WAN_COMMON_INTERFACE_CONFIG_NAMESPACE_URI,
                                                         std::bind(&Service::onActionFinished,
                                                                   this,
                                                                   std::placeholders::_1,
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
