#include <CloudTableModel.hpp>

CloudTableModel::CloudTableModel(QObject* parent /*= nullptr*/) {}

CloudTableModel::~CloudTableModel() {}

void CloudTableModel::appendCloud(const CloudProperty& cloud) {
    beginResetModel();
    clouds_.push_back(cloud);
    endResetModel();
}

void CloudTableModel::removeCloud(int id) {
    if (id < 0 || id >= clouds_.size())
        return;

    beginResetModel();
    clouds_.erase(clouds_.cbegin() + id);
    endResetModel();
}

const CloudProperty& CloudTableModel::cloudAt(int id) const {
    if (id < 0 || id >= clouds_.size()) {
        static CloudProperty empty{};
        return empty;
    }

    return clouds_[id];
}

void CloudTableModel::clearCloud() {
    beginResetModel();
    clouds_.clear();
    endResetModel();
}

int CloudTableModel::rowCount(const QModelIndex&) const { return clouds_.size(); }

int CloudTableModel::columnCount(const QModelIndex&) const { return 3; }

QVariant CloudTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= clouds_.size())
        return {};

    switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0) {
                return clouds_[index.row()].visible;
            } else if (index.column() == 1) {
                return clouds_[index.row()].name;
            } else if (index.column() == 2) {
                return clouds_[index.row()].label;
            }
        default: break;
    }

    return {};
}

QVariant CloudTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    static QStringList header_names{tr("visible"), tr("name"), tr("label")};
    switch (role) {
        case Qt::DisplayRole:
            if (orientation == Qt::Horizontal && section < header_names.count())
                return header_names.at(section);
            return section + 1;
        default: break;
    }

    return {};
}
