#include "upnp/DeviceFinder.hpp"

#include "upnp/Device.hpp"
#include "upnp/DeviceBuilder.hpp"
#include "upnp/Service.hpp"
#include "upnp/ServiceBuilder.hpp"

#include "util.hpp"

#include <QtCore/QDebug>
#include <QtCore/QXmlStreamReader>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSslError>

#include <algorithm>
#include <memory>
#include <vector>

namespace fritzmon {
namespace upnp {

using namespace internal;

static constexpr auto DESCRIPTION_NAMESPACE_URI = "urn:schemas-upnp-org:device-1-0";
static constexpr auto ROOT_TAG = "root";
static constexpr auto DEVICE_TAG = "device";
static constexpr auto DEVICETYPE_TAG = "deviceType";
static constexpr auto FRIENDLYNAME_TAG = "friendlyName";
static constexpr auto MANUFACTURER_TAG = "manufacturer";
static constexpr auto MANUFACTURERURL_TAG = "manufacturerURL";
static constexpr auto MODELDESCRIPTION_TAG = "modelDescription";
static constexpr auto MODELNAME_TAG = "modelName";
static constexpr auto MODELNUMBER_TAG = "modelNumber";
static constexpr auto MODELURL_TAG = "modelURL";
static constexpr auto SERIALNUMBER_TAG = "serialNumber";
static constexpr auto UDN_TAG = "udn";
static constexpr auto UPC_TAG = "upc";
static constexpr auto SERVICE_TAG = "service";
static constexpr auto SERVICETYPE_TAG = "serviceType";
static constexpr auto SERVICEID_TAG = "serviceId";
static constexpr auto SCPDURL_TAG = "SCPDURL";
static constexpr auto CONTROLURL_TAG = "controlURL";
static constexpr auto EVENTSUBURL_TAG = "eventSubURL";

DeviceFinder::DeviceFinder(QObject *parent)
  : QObject(parent),
    m_networkAccess(),
    m_devices(),
    m_baseURL(),
    m_searching(false)
{
    connect(&m_networkAccess, &QNetworkAccessManager::finished,
            this,             &DeviceFinder::deviceDescriptionReceived);
    connect(&m_networkAccess, &QNetworkAccessManager::sslErrors,
            this,             &DeviceFinder::onSslErrors);
}

DeviceFinder::~DeviceFinder() = default;

void DeviceFinder::cancelFind()
{
    if (!m_searching)
        return;
    m_baseURL.clear();
}

void DeviceFinder::findDevice(const QUrl &descriptionDocumentURL)
{
    if (m_searching) {
        qDebug() << "DeviceFinder: already searching";

        return;
    }

    m_baseURL.clear();
    m_baseURL.setScheme(descriptionDocumentURL.scheme());
    m_baseURL.setAuthority(descriptionDocumentURL.authority());
    m_networkAccess.get(QNetworkRequest(descriptionDocumentURL));
}

const std::vector<std::unique_ptr<Device> > &DeviceFinder::devices() const
{
    return m_devices;
}

bool DeviceFinder::searching() const
{
    return m_searching;
}

void DeviceFinder::deviceDescriptionReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "DeviceFinder::deviceDescriptionReceived: network error:"
                 << reply->errorString();

        return;
    }

    parseDeviceDescription(reply->readAll());
}

void DeviceFinder::onDeviceFinished()
{
    auto *sender = qobject_cast<DeviceBuilder *>(QObject::sender());
    auto builder = removeSmartpointerFromVector(m_deviceBuilders, sender);

    if (builder) {
        // Create a device from the device builder, the smartpointer takes care of the builder instance
        m_devices.emplace_back(builder->create());
        emit deviceAdded(m_devices.back().get());
    } else
        qDebug() << "DeviceFinder::onDeviceFinished: device not in builder pool";

    // If no devices are under construction (typically waiting for their service descriptions), then
    // inform the clients the search is completed
    if (m_deviceBuilders.empty()) {
        m_searching = false;
        emit searchComplete();
    }
}

void DeviceFinder::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    for (const auto &error : errors)
        qDebug() << "DeviceFinder::onSslErrors:" << error.errorString();

    if ((errors.size() == 1) && (errors[0].error() == QSslError::SelfSignedCertificate)) {
        qDebug() << "DeviceFinder::onSslErrors: self signed certificate (ignored)";
        reply->ignoreSslErrors();
    }
}

void DeviceFinder::parseDeviceDescription(const QByteArray &data)
{
    enum class ParserState {
        TopLevelElement,
        Device,
        Service
    } state = ParserState::TopLevelElement;
    QXmlStreamReader reader(data);
    std::vector<std::unique_ptr<DeviceBuilder>> deviceStack;
    std::unique_ptr<ServiceBuilder> service;

    if (reader.atEnd()) {
        qDebug() << "DeviceFinder: empty description document";

        return;
    }
    // start reading
    reader.readNext();
    // read root element and "verify" schema
    reader.readNext();
    if (reader.namespaceUri() != DESCRIPTION_NAMESPACE_URI) {
        qDebug() << "DeviceFinder: parse error: wrong root element namespace"
                 << reader.namespaceUri();

        return;
    }
    if (reader.name() != ROOT_TAG) {
        qDebug() << "DeviceFinder: parse error: wrong root element name" << reader.name();

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
            if ((tokenType == QXmlStreamReader::StartElement)
                    && (tagName == DEVICE_TAG)) {
                deviceStack.emplace_back(new DeviceBuilder());
                state = ParserState::Device;
            }
            break;
        case ParserState::Device:
            if (tokenType == QXmlStreamReader::StartElement) {
                auto &device = deviceStack.back();

                if (tagName == DEVICETYPE_TAG)
                    device->type(reader.readElementText());
                if (tagName == FRIENDLYNAME_TAG)
                    device->friendlyName(reader.readElementText());
                else if (tagName == MANUFACTURER_TAG)
                    device->manufacturerName(reader.readElementText());
                else if (tagName == MANUFACTURERURL_TAG)
                    device->manufacturerURL(reader.readElementText());
                else if (tagName == MODELDESCRIPTION_TAG)
                    device->description(reader.readElementText());
                else if (tagName == MODELNAME_TAG)
                    device->modelName(reader.readElementText());
                else if (tagName == MODELNUMBER_TAG)
                    device->modelNumber(reader.readElementText());
                else if (tagName == MODELURL_TAG)
                    device->modelURL(reader.readElementText());
                else if (tagName == SERIALNUMBER_TAG)
                    device->serialNumber(reader.readElementText());
                else if (tagName == UDN_TAG)
                    device->uniqueDeviceName(reader.readElementText());
                else if (tagName == UPC_TAG)
                    device->upc(reader.readElementText());
                else if (tagName == SERVICE_TAG) {
                    service.reset(new ServiceBuilder());
                    state = ParserState::Service;
                } else if (tagName == DEVICE_TAG)
                    deviceStack.emplace_back(new DeviceBuilder());
            } else if (tokenType == QXmlStreamReader::EndElement) {
                if (tagName == DEVICE_TAG) {
                    auto prototype = std::unique_ptr<DeviceBuilder>();

                    deviceStack.back().swap(prototype);
                    deviceStack.pop_back();
                    if (deviceStack.size() == 0) {
                        m_deviceBuilders.emplace_back(std::move(prototype));

                        auto result = connect(m_deviceBuilders.back().get(),
                                              &DeviceBuilder::finished,
                                              this, &DeviceFinder::onDeviceFinished);
                        Q_ASSERT(result);
                        Q_UNUSED(result);
                        state = ParserState::TopLevelElement;
                    } else
                        deviceStack.back()->addChild(std::move(prototype));
                }
            }
            break;
        case ParserState::Service:
            if (tokenType == QXmlStreamReader::StartElement) {
                if (tagName == SERVICETYPE_TAG)
                    service->type(reader.readElementText());
                else if (tagName == SERVICEID_TAG)
                    service->id(reader.readElementText());
                else if (tagName == SCPDURL_TAG) {
                    auto url = QUrl(reader.readElementText());

                    url.setScheme(m_baseURL.scheme());
                    url.setAuthority(m_baseURL.authority());
                    service->scpdURL(url);
                } else if (tagName == CONTROLURL_TAG) {
                    auto url = QUrl(reader.readElementText());

                    url.setScheme(m_baseURL.scheme());
                    url.setAuthority(m_baseURL.authority());
                    service->controlURL(url);
                } else if (tagName == EVENTSUBURL_TAG) {
                    auto url = QUrl(reader.readElementText());

                    url.setScheme(m_baseURL.scheme());
                    url.setAuthority(m_baseURL.authority());
                    service->eventSubURL(url);
                } else
                    qDebug() << "Service: unhandled tag:" << reader.name();
            } else if ((tokenType == QXmlStreamReader::EndElement) && (tagName == SERVICE_TAG)) {
                auto builder = std::unique_ptr<ServiceBuilder>();

                service.swap(builder);
                deviceStack.back()->addService(std::move(builder));
                state = ParserState::Device;
            }
            break;
        }
    }
    if (reader.hasError()) {
        qDebug() << "DeviceFinder: parse error: " << reader.errorString();

        return;
    }
}

} // namespace upnp
} // namespace fritzmon
