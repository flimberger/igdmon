/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef FRITZMON_GRAPH_HPP
#define FRITZMON_GRAPH_HPP

#include <QtCore/QAbstractListModel>

#include <QtGui/QColor>

#include <QtQuick/QQuickItem>

class QRectF;
class QSGNode;

namespace fritzmon {

class Graph : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor
               READ backgroundColor
               WRITE setBackgroundColor
               NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QVariant model READ model WRITE setModel NOTIFY modelChanged)

public:
    Graph(QQuickItem *parent=nullptr);
    ~Graph();

    void setModel(const QVariant &newModel);
    QVariant model() const;

    void setBackgroundColor(const QColor &newColor);
    QColor backgroundColor() const;

    void setColor(const QColor &newColor);
    QColor color() const;

Q_SIGNALS:
    void backgroundColorChanged(const QColor &newColor);
    void colorChanged(const QColor &newColor);
    void modelChanged();

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;

private:
    Q_SLOT void onSampleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                    const QVector<int> &roles);

    QAbstractListModel *m_samplesModel;
    QColor m_color;
    QColor m_backgroundColor;
    bool m_geometryChanged;
    bool m_samplesChanged;

    Q_DISABLE_COPY(Graph)
};

} // namespace fritzmon

#endif // FRITZMON_GRAPH_HPP
