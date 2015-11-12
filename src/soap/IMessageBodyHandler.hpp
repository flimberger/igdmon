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
    virtual bool startMessage() = 0; //< This should reset the handlers internal state
    virtual bool endElement(const QString &tag) = 0;
    virtual bool endMessage() = 0; //< This should finalize the handler, e.g. notify observers

private:
    QString m_namespaceURI;
};

} // namespace soap
} // namespace fritzmon

#endif // FRITZMON_SOAP_IMESSAGEBODYHANDLER_HPP
