#ifndef FRITZMON_GRAPHMODEL_HPP
#define FRITZMON_GRAPHMODEL_HPP

#include <QtCore/QAbstractListModel>

#include <vector>

namespace fritzmon {

class GraphModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GraphModel(QObject *parent=nullptr);

    void addSample(float value);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    std::vector<float> m_data;

    Q_DISABLE_COPY(GraphModel)
};

} // namespace fritzmon

#endif // FRITZMON_GRAPHMODEL_HPP
