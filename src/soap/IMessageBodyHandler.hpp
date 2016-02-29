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
