#include "Action.hpp"

namespace fritzmon {
namespace upnp {

Argument::Argument() = default;

Argument::Argument(const QString &name, const QString &stateVariable, Direction direction)
  : m_name(name),
    m_stateVariable(stateVariable),
    m_direction(direction)
{}

QString Argument::name() const
{
    return m_name;
}

QString Argument::stateVariable() const
{
    return m_stateVariable;
}

Argument::Direction Argument::direction() const
{
    return m_direction;
}

Action::Action() = default;

Action::Action(const QString &name, const std::vector<Argument> &arguments)
  : m_name(name),
    m_arguments(arguments)
{}

QString Action::name() const
{
    return m_name;
}

const std::vector<Argument> &Action::arguments() const
{
    return m_arguments;
}

} // namespace upnp
} // namespace fritzmon
