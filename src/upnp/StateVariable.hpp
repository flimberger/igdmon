#ifndef FRITZMON_UPNP_STATEVARIABLE_HPP
#define FRITZMON_UPNP_STATEVARIABLE_HPP

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace fritzmon {
namespace upnp {

class StateVariable
{
public:
    enum class Type {
        Uint1,
        Uint2,
        Uint4,
        Int1,
        Int2,
        Int4,
        Int,
        Real4,
        Real8,
        Number,
        Fixed_14_4,
        Float,
        Char,
        String,
        Date,
        Datetime,
        Datetime_tz,
        Time,
        Time_tz,
        Boolean,
        Bin_Base64,
        Bin_Hex,
        Uri,
        Uuid
    };

    StateVariable(const QString &name, Type type, const QVariant &value);

    QString name() const;
    Type type() const;
    QVariant value() const;

private:
    QString m_name;
    Type m_type;
    QVariant m_value;
};

} // namespace upnp
} // namespace fritzmon

#endif // FRITZMON_UPNP_STATEVARIABLE_HPP
