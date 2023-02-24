#pragma once
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>

#include <QAbstractTableModel>
#include <QStringList>
#include <vector>

struct CloudProperty {
    QString name;
    int category{};
    bool visible{};
    vtkSmartPointer<vtkCellArray> vertices{};
};

class CloudTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit CloudTableModel(QObject* parent = nullptr);
    ~CloudTableModel();

    void appendCloud(const CloudProperty& cloud);
    void removeCloud(int id);
    bool setCloud(int id, const CloudProperty& cloud);
    bool setCloudCategory(int id, int category);
    const CloudProperty& cloudAt(int id) const;
    void clearCloud();
    void setCategoryNames(const QStringList& names);
    const QVector<CloudProperty>& cloudItems() const;
    QString categoryName(int category) const;

    // override
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QVector<CloudProperty> clouds_;
    QStringList category_names_;
};
