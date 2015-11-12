#include "IMessageBodyHandler.hpp"

namespace fritzmon {
namespace soap {

IMessageBodyHandler::IMessageBodyHandler(const QString &namespaceURI)
  : m_namespaceURI(namespaceURI)
{}

IMessageBodyHandler::~IMessageBodyHandler() = default;

QString IMessageBodyHandler::namespaceURI() const
{
    return m_namespaceURI;
}

} // namespace soap
} // namespace fritzmon
