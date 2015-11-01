#include "GraphModel.hpp"

namespace fritzmon {

GraphModel::GraphModel(QObject *parent)
  : QAbstractListModel(parent),
    m_data()
{}

void GraphModel::addSample(float value)
{
    m_data.push_back(value);

    auto index = createIndex(m_data.size() - 1, 0);

    emit dataChanged(index, index, {});
}

int GraphModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

QVariant GraphModel::data(const QModelIndex &index, int role) const
{
    return m_data[index.row()];
}

} // namespace fritzmon
