/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "WebNodeCollection.h"

#include "core/dom/Node.h"
#include "core/html/HTMLCollection.h"
#include "wtf/PassRefPtr.h"

#include "WebNode.h"

using namespace WebCore;

namespace WebKit {

void WebNodeCollection::reset()
{
    assign(0);
}

void WebNodeCollection::assign(const WebNodeCollection& other)
{
    HTMLCollection* p = const_cast<HTMLCollection*>(other.m_private);
    if (p)
        p->ref();
    assign(p);
}

WebNodeCollection::WebNodeCollection(const PassRefPtr<HTMLCollection>& col)
    : m_private(static_cast<HTMLCollection*>(col.leakRef()))
{
}

void WebNodeCollection::assign(HTMLCollection* p)
{
    // p is already ref'd for us by the caller
    if (m_private)
        m_private->deref();
    m_private = p;
}

unsigned WebNodeCollection::length() const
{
    return m_private->length();
}

WebNode WebNodeCollection::nextItem() const
{
    Node* node = m_private->item(m_current);
    if (node)
        m_current++;
    return WebNode(node);
}

WebNode WebNodeCollection::firstItem() const
{
    m_current = 0;
    return nextItem();
}

} // namespace WebKit
