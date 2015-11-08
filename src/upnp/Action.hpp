#ifndef UPNP_ACTION_HPP
#define UPNP_ACTION_HPP

#include <QtCore/QString>

#include <vector>

namespace fritzmon {
namespace upnp {

class Argument
{
public:
    enum class Direction {
        In,
        Out
    };

    Argument();
    explicit Argument(const QString &name, const QString &stateVariable, Direction direction);

    QString name() const;
    QString stateVariable() const;
    Direction direction() const;

private:
    QString m_name;
    QString m_stateVariable;
    Direction m_direction;
};

class Action
{
public:
    Action();
    explicit Action(const QString &name, const std::vector<Argument> &arguments);

    QString name() const;
    const std::vector<Argument> &arguments() const;

private:
    QString m_name;
    std::vector<Argument> m_arguments;
};

} // namespace upnp
} // namespace fritzmon

#endif // UPNP_ACTION_HPP
