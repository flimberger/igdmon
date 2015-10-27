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
    m_instance(new Service()),
    m_type(),
    m_id(),
    m_scpdURL(),
    m_controlURL(),
    m_eventSubURL(),
    m_done(false)
{
    auto result = connect(&m_networkAccess, &QNetworkAccessManager::finished,
                          this,             &ServiceBuilder::serviceDescriptionReceived);
    Q_ASSERT(result);
    Q_UNUSED(result);
}

ServiceBuilder::~ServiceBuilder() = default;

ServiceBuilder &ServiceBuilder::type(const QString &serviceType)
{
    m_type = serviceType;
    return *this;
}

ServiceBuilder &ServiceBuilder::id(const QString &serviceId)
{
    m_id = serviceId;
    return *this;
}

ServiceBuilder &ServiceBuilder::scpdURL(const QUrl &scpdURL)
{
    m_scpdURL = scpdURL;
    return *this;
}

ServiceBuilder &ServiceBuilder::controlURL(const QUrl &controlURL)
{
    m_controlURL = controlURL;
    return *this;
}

ServiceBuilder &ServiceBuilder::eventSubURL(const QUrl &eventURL)
{
    m_eventSubURL = eventURL;
    return *this;
}

void ServiceBuilder::startDetection()
{
    Q_ASSERT(!m_scpdURL.isEmpty());

    m_networkAccess.get(QNetworkRequest(m_scpdURL));
}

std::unique_ptr<Service> ServiceBuilder::create()
{
    auto ptr = std::unique_ptr<Service>();

    m_instance.swap(ptr);

    return ptr;
}

bool ServiceBuilder::done() const
{
    return m_done;
}

void ServiceBuilder::serviceDescriptionReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "ServiceBuilder: network error:" << reply->errorString();

        return;
    }
    parseServiceDescription(reply->readAll());
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
    QVector<Argument> actionArguments;
    QString argumentName;
    QString argumentStateVariable;
    Argument::Direction argumentDirection;

    if (reader.atEnd()) {
        qDebug() << "ServiceBuilder: empty description document";
        goto fail;
    }
    // start reading
    reader.readNext();
    // read root element and "verify" schema
    reader.readNext();
    if (reader.namespaceUri() != DESCRIPTION_NAMESPACE_URI) {
        qDebug() << "ServiceBuilder: parse error: wrong root element namespace"
                 << reader.namespaceUri();
        goto fail;
    }
    if (reader.name() != ROOT_TAG) {
        qDebug() << "ServiceBuilder: parse error: wrong root element name" << reader.name();
        goto fail;
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
                    m_instance->m_actions.push_back(Action(actionName, actionArguments));
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
                actionArguments.push_back(Argument(argumentName, argumentStateVariable,
                                                   argumentDirection));
                argumentName.clear();
                argumentStateVariable.clear();
                state = ParserState::Action;
            }
        }
    }
    if (reader.hasError()) {
        qDebug() << "ServiceBuilder: parse error: " << reader.errorString();
        goto fail;
    }
    m_done = true;
    emit serviceDetected(this);

    return;

fail:
    m_done = true;
    emit serviceDetectionFailed(this);
}

} // namespace internal
} // namespace upnp
} // namespace fritzmon
