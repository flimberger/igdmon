#ifndef FRITZMON_SOAP_IMESSAGEBODYHANDLER_HPP
#define FRITZMON_SOAP_IMESSAGEBODYHANDLER_HPP

#include <QtCore/QString>

class QXmlStreamReader;

namespace fritzmon {
namespace soap {

class IMessageBodyHandler
{
public:
    IMessageBodyHandler(const QString &namespaceURI);
    ~IMessageBodyHandler();

    QString namespaceURI() const;

    virtual bool startElement(const QString &tag, QXmlStreamReader &stream) = 0;
    virtual bool endElement(const QString &tag) = 0;

private:
    QString m_namespaceURI;
};

} // namespace soap
} // namespace fritzmon

#endif // FRITZMON_SOAP_IMESSAGEBODYHANDLER_HPP
