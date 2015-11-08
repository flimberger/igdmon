#include "ServiceBuilder.hpp"

#include "upnp/Action.hpp"
#include "upnp/Service.hpp"

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

ServiceBuilder::ServiceBuilder(QObject *parent)
  : QObject(parent),
    m_networkAccess(),
    m_instance(new Service())
{
    auto result = connect(&m_networkAccess, &QNetworkAccessManager::finished,
                          this,             &ServiceBuilder::serviceDescriptionReceived);
    Q_ASSERT(result);
    Q_UNUSED(result);
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

void ServiceBuilder::serviceDescriptionReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "ServiceBuilder: network error:" << reply->errorString();

        return;
    }
    parseServiceDescription(reply->readAll());

    emit finished();
}

void ServiceBuilder::parseServiceDescription(const QByteArray &data)
{
    enum class ParserState {
        TopLevelElement,
        Action,
        Argument
    } state = ParserState::TopLevelElement;
    QXmlStreamReader reader(data);

    // TODO: yeah...
    QString actionName;
    std::vector<Argument> actionArguments;
    QString argumentName;
    QString argumentStateVariable;
    Argument::Direction argumentDirection;

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
            if ((tokenType == QXmlStreamReader::StartElement) && (tagName == ACTION_TAG))
                state = ParserState::Action;
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
        }
    }
    if (reader.hasError()) {
        qDebug() << "ServiceBuilder: parse error: " << reader.errorString();
    }
}

} // namespace internal
} // namespace upnp
} // namespace fritzmon
