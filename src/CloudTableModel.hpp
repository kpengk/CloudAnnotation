#pragma once
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>

#include <QAbstractTableModel>
#include <vector>

struct CloudProperty {
    QString name;
    bool visible;
    int label;
    vtkSmartPointer<vtkCellArray> vertices;
};

class CloudTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit CloudTableModel(QObject* parent = nullptr);
    ~CloudTableModel();

    void appendCloud(const CloudProperty& cloud);
    void removeCloud(int id);
    const CloudProperty& cloudAt(int id) const;
    void clearCloud();

    // override
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:

private:
    std::vector<CloudProperty> clouds_;
};
