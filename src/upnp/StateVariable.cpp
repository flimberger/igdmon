#include "StateVariable.hpp"

namespace fritzmon {
namespace upnp {

StateVariable::StateVariable(const QString &name, Type type, const QVariant &value)
  : m_name(name),
    m_type(type),
    m_value(value)
{}

QString StateVariable::name() const
{
    return m_name;
}

StateVariable::Type StateVariable::type() const
{
    return m_type;
}

QVariant StateVariable::value() const
{
    return m_value;
}

} // namespace upnp
} // namespace fritzmon
