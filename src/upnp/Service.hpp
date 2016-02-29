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

#ifndef UPNP_SERVICE_HPP
#define UPNP_SERVICE_HPP

#include <upnp/Action.hpp>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVariantMap>

#include <memory>
#include <vector>

namespace fritzmon {

namespace soap {

class Request;

} // namespace soap

namespace upnp {

class StateVariable;

namespace internal {

class ServiceBuilder;

} // namespace internal

class Service : public QObject
{
    Q_OBJECT

public:
    enum class InvokeActionResult {
        Success,
        PendingAction,    //< There is already an invocation pending
        InvocationFailed, //< The invocation failed
        InvalidAction     //< no such action available from this service
    };

    explicit Service(QObject *parent=nullptr);
    ~Service();

    InvokeActionResult invokeAction(const QString &name, const QVariantMap &inputArguments);
    void queryStateVariable(const QString &name, QVariant &value);

    QString id() const;
    QString serviceTypeIdentifier() const;

    const std::vector<Action> &actions() const;
    const std::vector<StateVariable> &stateVariables() const;

Q_SIGNALS:
    void actionInvoked(const QVariantMap &outputArguments, const QVariant &returnValue);
    void serviceInstanceDied();
    void stateVariableChanged(const QString &name, const QVariant &value);

private:
    /* Q_SLOT */ void onActionFinished(const QVariantMap &outputArguments,
                                       const QVariant &returnValue);

    std::vector<Action> m_actions;
    std::vector<StateVariable> m_stateVariables;
    QString m_type;
    QString m_id;
    QUrl m_scpdURL;
    QUrl m_controlURL;
    QUrl m_eventSubURL;
    std::unique_ptr<soap::Request> m_request;
    bool m_invokationPending;

    friend class internal::ServiceBuilder;
    Q_DISABLE_COPY(Service)
};

} // namespace upnp
} // namespace fritzmon

#endif // UPNP_SERVICE_HPP
