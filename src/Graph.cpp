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

#include "Graph.hpp"

#include <QtCore/QDebug>
#include <QtCore/QRectF>

#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGGeometry>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGNode>
#include <QtQuick/QSGSimpleMaterial>
#include <QtQuick/QSGTexture>

#include <memory>
#include <vector>

namespace fritzmon {

// background

static constexpr auto NOISE_SIZE = 64;

class BackgroundNode : public QSGGeometryNode
{
public:
    BackgroundNode(QQuickWindow *window, const QColor &color);

    void setRect(const QRectF &bounds);
};

struct NoisyMaterial
{
    ~NoisyMaterial() {
        if (texture)
            delete texture;
    }

    QColor color;
    QSGTexture *texture; // a smartpointer is not understood by the API
};

class NoisyShader : public QSGSimpleMaterialShader<NoisyMaterial>
{
    QSG_DECLARE_SIMPLE_SHADER(NoisyShader, NoisyMaterial)

public:
    NoisyShader() {
        setShaderSourceFile(QOpenGLShader::Vertex, ":/fritzmon/shaders/noisy.vsh");
        setShaderSourceFile(QOpenGLShader::Fragment, ":/fritzmon/shaders/noisy.fsh");
    }

    QList<QByteArray> attributes() const override {
        return QList<QByteArray>() << "aVertex" << "aTexCoord";
    }

    void updateState(const NoisyMaterial *m, const NoisyMaterial *data) override {
        program()->setUniformValue(m_idColor, m->color);
        m->texture->bind();

        auto s = m->texture->textureSize();

        program()->setUniformValue(m_idTextureSize, QSizeF(1.0f / s.width(), 1.0f / s.height()));
    }

    virtual void resolveUniforms() override {
        m_idColor = program()->uniformLocation("color");
        m_idTexture = program()->uniformLocation("texture");
        m_idTextureSize = program()->uniformLocation("textureSize");

        program()->setUniformValue(m_idTexture, 0);
    }

private:
    int m_idColor;
    int m_idTexture;
    int m_idTextureSize;
};

BackgroundNode::BackgroundNode(QQuickWindow *window, const QColor &color)
  : QSGGeometryNode()
{
    auto image = QImage(NOISE_SIZE, NOISE_SIZE, QImage::Format_RGB32);
    auto *data = reinterpret_cast<uint *>(image.bits());

    for (int i = 0; i < NOISE_SIZE * NOISE_SIZE; ++i) {
        auto g = static_cast<uint>(rand()) & 0xff;
        data[i] = 0xff000000 | (g << 16) | (g << 8) | g;
    }

    auto *t = window->createTextureFromImage(image);

    t->setFiltering(QSGTexture::Nearest);
    t->setHorizontalWrapMode(QSGTexture::Repeat);
    t->setVerticalWrapMode(QSGTexture::Repeat);

    auto *m = NoisyShader::createMaterial();

    m->state()->texture = t;
    m->state()->color = color;
    m->setFlag(QSGMaterial::Blending);
    setMaterial(m);
    setFlag(OwnsMaterial, true);

    auto *g = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);

    QSGGeometry::updateTexturedRectGeometry(g, QRect(), QRect());
    setGeometry(g);
    setFlag(OwnsGeometry, true);
}

void BackgroundNode::setRect(const QRectF &bounds)
{
    QSGGeometry::updateTexturedRectGeometry(geometry(), bounds, QRectF(0, 0, 1, 1));
    markDirty(QSGNode::DirtyGeometry);
}

// Line

class LineNode : public QSGGeometryNode
{
public:
    LineNode(float size, float spread, const QColor &color);

    void updateGeometry(const QRectF &bounds, float upperBound, const std::vector<float> &samples);

private:
    QSGGeometry m_geometry;
};

struct LineMaterial
{
    QColor color;
    float spread;
    float size;
};

class LineShader : public QSGSimpleMaterialShader<LineMaterial>
{
    QSG_DECLARE_SIMPLE_SHADER(LineShader, LineMaterial)

public:
    LineShader() {
        setShaderSourceFile(QOpenGLShader::Vertex, ":/fritzmon/shaders/line.vsh");
        setShaderSourceFile(QOpenGLShader::Fragment, ":/fritzmon/shaders/line.fsh");
    }

    QList<QByteArray> attributes() const override {
        return QList<QByteArray>() << "pos" << "t";
    }

    void updateState(const LineMaterial *m, const LineMaterial *n) override {
        Q_UNUSED(n);

        program()->setUniformValue(m_idColor, m->color);
        program()->setUniformValue(m_idSpread, m->spread);
        program()->setUniformValue(m_idSize, m->size);
    }

    void resolveUniforms() override {
        m_idColor = program()->uniformLocation("color");
        m_idSpread = program()->uniformLocation("spread");
        m_idSize = program()->uniformLocation("size");
    }

private:
    int m_idColor;
    int m_idSpread;
    int m_idSize;
};

struct LineVertex {
    float x;
    float y;
    float t;

    inline void set(float vx, float vy, float vt) {
        x = vx;
        y = vy;
        t = vt;
    }
};

static const QSGGeometry::AttributeSet &attributes()
{
    static QSGGeometry::Attribute attr[] = {
        QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
        QSGGeometry::Attribute::create(1, 1, GL_FLOAT)
    };
    static QSGGeometry::AttributeSet set = { 2, 3 * sizeof(float), attr };

    return set;
}

LineNode::LineNode(float size, float spread, const QColor &color)
  : QSGGeometryNode(),
    m_geometry(attributes(), 0)
{
    setGeometry(&m_geometry);
    m_geometry.setDrawingMode(GL_TRIANGLE_STRIP);

    auto *m = LineShader::createMaterial();

    m->state()->color = color;
    m->state()->spread = spread;
    m->state()->size = size;
    m->setFlag(QSGMaterial::Blending);
    setMaterial(m);
    setFlag(OwnsMaterial);
}

void LineNode::updateGeometry(const QRectF &bounds, float upperBound,
                              const std::vector<float> &samples)
{
    m_geometry.allocate(samples.size() * 2);

    auto x = bounds.x();
    auto w = bounds.width();
    auto h = bounds.height();
    auto dx = w / (samples.size() - 1);
    auto dy = h / upperBound;
    auto *vertex = static_cast<LineVertex *>(m_geometry.vertexData());

    for (int i = 0; i < samples.size(); ++i) {
        auto ix = x + dx * i;
        auto iy = h - dy * samples[i];

        vertex[i * 2].set(ix, iy, 0);
        vertex[i * 2 + 1].set(ix, iy, 1);
    }

    markDirty(QSGNode::DirtyGeometry);
}

// graph

class GraphNode : public QSGNode
{
public:
    BackgroundNode *background;
    LineNode *line;
};

Graph::Graph(QQuickItem *parent)
  : QQuickItem(parent),
    m_samplesModel(nullptr),
    m_color(QColor("#ff9900")),
    m_backgroundColor(QColor("#333333")),
    m_upperBound(10.0f),
    m_geometryChanged(false),
    m_samplesChanged(false)
{
    setFlag(ItemHasContents, true);
}

Graph::~Graph()
{
    if (m_samplesModel && (m_samplesModel->parent() == this))
        delete m_samplesModel;
}

void Graph::setBackgroundColor(const QColor &newColor)
{
    m_color = newColor;
    emit backgroundColorChanged(newColor);
}

QColor Graph::backgroundColor() const
{
    return m_backgroundColor;
}

void Graph::setColor(const QColor &newColor)
{
    m_color = newColor;
    emit colorChanged(newColor);
}

QColor Graph::color() const
{
    return m_color;
}

void Graph::setModel(const QVariant &newModel)
{
    auto modelptr = newModel.value<QAbstractListModel *>();

    if (!modelptr) {
        qDebug() << "Graph::setModel: nullptr received";
    }

    // if the model has no parent, take ownership by assuming parenthood
    if (modelptr && (modelptr->parent() == nullptr))
        modelptr->setParent(this);
    if (m_samplesModel) {
        disconnect(m_samplesModel, &QAbstractListModel::dataChanged,
                   this,           &Graph::onSampleDataChanged);
        if (m_samplesModel->parent() == this)
            delete m_samplesModel;
    }
    m_samplesModel = modelptr;
    if (m_samplesModel)
        connect(m_samplesModel, &QAbstractListModel::dataChanged,
                this,           &Graph::onSampleDataChanged);
    emit modelChanged();

    m_samplesChanged = true;
    update();
}

QVariant Graph::model() const
{
    return QVariant::fromValue(m_samplesModel);
}

void Graph::setUpperBound(float newUpperBound)
{
    m_upperBound = newUpperBound;

    emit upperBoundChanged(newUpperBound);

    m_samplesChanged = true;
    update();
}

float Graph::upperBound() const
{
    return m_upperBound;
}

void Graph::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    m_geometryChanged = true;
    update();
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

QSGNode *Graph::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);

    auto nodeptr = std::unique_ptr<GraphNode>(static_cast<GraphNode *>(node));
    auto bounds = boundingRect();

    if (bounds.isEmpty())
        return nullptr;
    if (!nodeptr) {
        nodeptr = std::make_unique<GraphNode>();
        // those objects are managed by the QObject hierarchy, so no smartpointers are used
        nodeptr->background = new BackgroundNode(window(), m_backgroundColor);
        nodeptr->line = new LineNode(10.0f, 0.8f, m_color);
        nodeptr->appendChildNode(nodeptr->background);
        nodeptr->appendChildNode(nodeptr->line);
    }
    if (m_geometryChanged) {
        nodeptr->background->setRect(bounds);
    }
    if (m_samplesModel && (m_geometryChanged || m_samplesChanged)) {
        // bruteforce approach
        auto rowCount = m_samplesModel->rowCount();
        auto samples = std::vector<float>(rowCount);

        for (auto i = 0; i < rowCount; ++i)
            samples[i] = m_samplesModel->data(m_samplesModel->index(i)).value<float>();
        nodeptr->line->updateGeometry(bounds, m_upperBound, samples);
    }
    m_geometryChanged = false;
    m_samplesChanged = false;

    // stop managing the object
    return nodeptr.release();
}

void Graph::onSampleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                const QVector<int> &roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);

    m_samplesChanged = true;
    update();
}

} // namespace fritzmon
