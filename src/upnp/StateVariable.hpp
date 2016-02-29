/*
 * Copyright (c) 2016 Florian Limberger <flo@snakeoilproductions.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
