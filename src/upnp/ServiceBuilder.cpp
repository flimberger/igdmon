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

#include "ServiceBuilder.hpp"

#include "upnp/Action.hpp"
#include "upnp/Service.hpp"
#include "upnp/StateVariable.hpp"

#include <QtCore/QDebug>
#include <QtCore/QXmlStreamReader>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace fritzmon {
namespace upnp {
namespace internal {

static constexpr auto DESCRIPTION_NAMESPACE_URI = "urn:schemas-upnp-org:service-1-0";
static constexpr auto ROOT_TAG = "scpd";
static constexpr auto ACTION_TAG = "action";
static constexpr auto NAME_TAG = "name";
static constexpr auto ARGUMENT_TAG = "argument";
static constexpr auto DIRECTION_TAG = "direction";
static constexpr auto DIRECTION_IN = "in";
static constexpr auto DIRECTION_OUT = "out";
static constexpr auto RELATEDSTATEVARIABLE_TAG = "relatedStateVariable";
static constexpr auto *STATE_VARIABLE = "stateVariable";
static constexpr auto *DATA_TYPE = "dataType";
static constexpr auto *VALUE_UINT1 = "ui1";
static constexpr auto *VALUE_UINT2 = "ui2";
static constexpr auto *VALUE_UINT4 = "ui4";
static constexpr auto *VALUE_INT1 = "i1";
static constexpr auto *VALUE_INT2 = "i2";
static constexpr auto *VALUE_INT4 = "i4";
static constexpr auto *VALUE_INT = "int";
static constexpr auto *VALUE_REAL4 = "r4";
static constexpr auto *VALUE_REAL8 = "r8";
static constexpr auto *VALUE_NUMBER = "number";
static constexpr auto *VALUE_FIXED_14_4 = "fixed.14.4";
static constexpr auto *VALUE_FLOAT = "float";
static constexpr auto *VALUE_CHAR = "char";
static constexpr auto *VALUE_STRING = "string";
static constexpr auto *VALUE_DATE = "date";
static constexpr auto *VALUE_DATETIME = "datetime";
static constexpr auto *VALUE_DATETIME_TZ = "datetime.tz";
static constexpr auto *VALUE_TIME = "time";
static constexpr auto *VALUE_TIME_TZ = "time.tz";
static constexpr auto *VALUE_BOOLEAN = "boolean";
static constexpr auto *VALUE_BIN_BASE64 = "bin.base64";
static constexpr auto *VALUE_BIN_HEX = "bin.hex";
static constexpr auto *VALUE_URI = "uri";
static constexpr auto *VALUE_UUID = "uuid";
static constexpr auto *ALLOWED_VALUE = "allowedValue";
static constexpr auto *DEFAULT_VALUE = "defaultValue";
static constexpr auto *ALLOWED_VALUE_RANGE = "allowedValueRange";
static constexpr auto *VALUE_MINIMUM = "minimum";
static constexpr auto *VALUE_MAXIMUM = "maximum";
static constexpr auto *VALUE_STEP = "step";

ServiceBuilder::ServiceBuilder(QObject *parent)
  : QObject(parent),
    m_networkAccess(),
    m_instance(new Service())
{
    connect(&m_networkAccess, &QNetworkAccessManager::finished,
            this,             &ServiceBuilder::serviceDescriptionReceived);
    connect(&m_networkAccess, &QNetworkAccessManager::sslErrors,
            this,             &ServiceBuilder::onSslErrors);
}

ServiceBuilder::~ServiceBuilder() = default;

ServiceBuilder &ServiceBuilder::type(const QString &serviceType)
{
    m_instance->m_type = serviceType;
    return *this;
}

ServiceBuilder &ServiceBuilder::id(const QString &serviceId)
{
    m_instance->m_id = serviceId;
    return *this;
}

ServiceBuilder &ServiceBuilder::scpdURL(const QUrl &scpdURL)
{
    m_instance->m_scpdURL = scpdURL;
    return *this;
}

ServiceBuilder &ServiceBuilder::controlURL(const QUrl &controlURL)
{
    m_instance->m_controlURL = controlURL;
    return *this;
}

ServiceBuilder &ServiceBuilder::eventSubURL(const QUrl &eventURL)
{
    m_instance->m_eventSubURL = eventURL;
    return *this;
}

void ServiceBuilder::startDetection()
{
    Q_ASSERT(!m_instance->m_scpdURL.isEmpty());

    m_networkAccess.get(QNetworkRequest(m_instance->m_scpdURL));
}

std::unique_ptr<Service> ServiceBuilder::create()
{
    auto ptr = std::unique_ptr<Service>();

    m_instance.swap(ptr);

    return ptr;
}

void ServiceBuilder::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    if ((errors.size() == 1) && (errors[0].error() == QSslError::SelfSignedCertificate)){
        qDebug() << "ServiceBuilder::onSslError: self signed certificate (ignored)";
        reply->ignoreSslErrors();
    }
}

void ServiceBuilder::serviceDescriptionReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
        qDebug() << "ServiceBuilder: network error:" << reply->errorString();
    else
        parseServiceDescription(reply->readAll());

    emit finished();
}

void ServiceBuilder::parseServiceDescription(const QByteArray &data)
{
    enum class ParserState {
        TopLevelElement,
        Action,
        Argument,
        StateVariable
    } state = ParserState::TopLevelElement;
    QXmlStreamReader reader(data);

    // TODO: yeah...
    QString actionName;
    std::vector<Argument> actionArguments;
    QString argumentName;
    QString argumentStateVariable;
    Argument::Direction argumentDirection;
    QString variableName;
    StateVariable::Type variableType;

    if (reader.atEnd()) {
        qDebug() << "ServiceBuilder: empty description document";

        return;
    }
    // start reading
    reader.readNext();
    // read root element and "verify" schema
    reader.readNext();
    if (reader.namespaceUri() != DESCRIPTION_NAMESPACE_URI) {
        qDebug() << "ServiceBuilder: parse error: wrong root element namespace"
                 << reader.namespaceUri();

        return;
    }
    if (reader.name() != ROOT_TAG) {
        qDebug() << "ServiceBuilder: parse error: wrong root element name" << reader.name();

        return;
    }
    // parse the description
    for (reader.readNext(); !reader.atEnd(); reader.readNext()) {
        // Don't check for the element namespace, just assume things are right for now.  This
        // behaviour may change if there are description documents with mixed xml schemas.

        const auto &tokenType = reader.tokenType();
        const auto &tagName = reader.name();

        switch (state) {
        case ParserState::TopLevelElement:
            if (tokenType == QXmlStreamReader::StartElement) {
                if (tagName == ACTION_TAG)
                    state = ParserState::Action;
                else if (tagName == STATE_VARIABLE)
                    state = ParserState::StateVariable;
            }
            break;
        case ParserState::Action:
            if (tokenType == QXmlStreamReader::StartElement) {
                if (tagName == NAME_TAG)
                    actionName = reader.readElementText();
                else if (tagName == ARGUMENT_TAG)
                    state = ParserState::Argument;
            } else if (tokenType == QXmlStreamReader::EndElement)
                if (tagName == ACTION_TAG) {
                    m_instance->m_actions.emplace_back(actionName, actionArguments);
                    actionName.clear();
                    actionArguments.clear();
                    state = ParserState::TopLevelElement;
                }
            break;
        case ParserState::Argument:
            if (tokenType == QXmlStreamReader::StartElement) {
                if (tagName == NAME_TAG)
                    argumentName = reader.readElementText();
                else if (tagName == RELATEDSTATEVARIABLE_TAG)
                    argumentStateVariable = reader.readElementText();
                else if (tagName == DIRECTION_TAG) {
                    auto text = reader.readElementText();
                    if (text == DIRECTION_IN)
                        argumentDirection = Argument::Direction::In;
                    else if (text == DIRECTION_OUT)
                        argumentDirection = Argument::Direction::Out;
                }
            } else if ((tokenType == QXmlStreamReader::EndElement) && (tagName == ARGUMENT_TAG)) {
                actionArguments.emplace_back(argumentName, argumentStateVariable,
                                             argumentDirection);
                argumentName.clear();
                argumentStateVariable.clear();
                state = ParserState::Action;
            }
            break;
        case ParserState::StateVariable:
            if (tokenType == QXmlStreamReader::StartElement) {
                if (tagName == NAME_TAG)
                    variableName = reader.readElementText();
                else if (tagName == DATA_TYPE) {
                    auto value = reader.readElementText();

                    if (value == VALUE_UINT1)
                        variableType = StateVariable::Type::Uint1;
                    else if (value == VALUE_UINT2)
                        variableType = StateVariable::Type::Uint2;
                    else if (value == VALUE_UINT4)
                        variableType = StateVariable::Type::Uint4;
                    else if (value == VALUE_INT1)
                        variableType = StateVariable::Type::Int1;
                    else if (value == VALUE_INT2)
                        variableType = StateVariable::Type::Int2;
                    else if (value == VALUE_INT4)
                        variableType = StateVariable::Type::Int4;
                    else if (value == VALUE_INT)
                        variableType = StateVariable::Type::Int;
                    else if (value == VALUE_REAL4)
                        variableType = StateVariable::Type::Real4;
                    else if (value == VALUE_REAL8)
                        variableType = StateVariable::Type::Real8;
                    else if (value == VALUE_NUMBER)
                        variableType = StateVariable::Type::Number;
                    else if (value == VALUE_FIXED_14_4)
                        variableType = StateVariable::Type::Fixed_14_4;
                    else if (value == VALUE_FLOAT)
                        variableType = StateVariable::Type::Float;
                    else if (value == VALUE_CHAR)
                        variableType = StateVariable::Type::Char;
                    else if (value == VALUE_STRING)
                        variableType = StateVariable::Type::String;
                    else if (value == VALUE_DATE)
                        variableType = StateVariable::Type::Date;
                    else if (value == VALUE_DATETIME)
                        variableType = StateVariable::Type::Datetime;
                    else if (value == VALUE_DATETIME_TZ)
                        variableType = StateVariable::Type::Datetime_tz;
                    else if (value == VALUE_TIME)
                        variableType = StateVariable::Type::Time;
                    else if (value == VALUE_TIME_TZ)
                        variableType = StateVariable::Type::Time_tz;
                    else if (value == VALUE_BOOLEAN)
                        variableType = StateVariable::Type::Boolean;
                    else if (value == VALUE_BIN_BASE64)
                        variableType = StateVariable::Type::Bin_Base64;
                    else if (value == VALUE_BIN_HEX)
                        variableType = StateVariable::Type::Bin_Hex;
                    else if (value == VALUE_URI)
                        variableType = StateVariable::Type::Uri;
                    else if (value == VALUE_UUID)
                        variableType = StateVariable::Type::Uuid;
                    else
                        qDebug() << "ServiceBuilder::parseServiceDescription: invalid value"
                                 << value;
                } else if (tagName == ALLOWED_VALUE) {
                    //
                } else if (tagName == DEFAULT_VALUE) {
                    //
                } else if (tagName == ALLOWED_VALUE_RANGE) {
                    //
                }
            } else if (tokenType == QXmlStreamReader::EndElement) {
                if (tagName == STATE_VARIABLE) {
                    m_instance->m_stateVariables.emplace_back(variableName, variableType,
                                                              QVariant());
                    state = ParserState::TopLevelElement;
                }
            }
            break;
        }
    }
    if (reader.hasError()) {
        qDebug() << "ServiceBuilder: parse error: " << reader.errorString();
    }
}

} // namespace internal
} // namespace upnp
} // namespace fritzmon
