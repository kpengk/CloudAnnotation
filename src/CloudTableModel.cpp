#include <CloudTableModel.hpp>

constexpr int name_column{0};
constexpr int category_column{1};
constexpr int visiable_column{2};

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

bool CloudTableModel::setCloud(int id, const CloudProperty& cloud) {
    if (id < 0 || id >= clouds_.size()) {
        return false;
    }

    beginResetModel();
    clouds_[id] = cloud;
    endResetModel();
    return true;
}

bool CloudTableModel::setCloudCategory(int id, int label) {
    if (id < 0 || id >= clouds_.size()) {
        return false;
    }

    beginResetModel();
    clouds_[id].category = label;
    endResetModel();
    return true;
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

void CloudTableModel::setCategoryNames(const QStringList& names) {
    beginResetModel();
    category_names_ = names;
    endResetModel();
}

const QVector<CloudProperty>& CloudTableModel::cloudItems() const { return clouds_; }

int CloudTableModel::rowCount(const QModelIndex&) const { return clouds_.size(); }

int CloudTableModel::columnCount(const QModelIndex&) const { return 2; }

QVariant CloudTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= clouds_.size())
        return {};

    switch (role) {
        case Qt::DisplayRole:
            if (index.column() == name_column) {
                return clouds_[index.row()].name;
            } else if (index.column() == category_column) {
                const int label = clouds_[index.row()].category;
                return categoryName(label);
            } else if (index.column() == visiable_column) {
                return clouds_[index.row()].visible;
            }
        default: break;
    }

    return {};
}

QVariant CloudTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    static QStringList header_names{tr("name"), tr("label"), tr("visible")};
    switch (role) {
        case Qt::DisplayRole:
            if (orientation == Qt::Horizontal && section < header_names.count())
                return header_names.at(section);
            return section + 1;
        default: break;
    }

    return {};
}

QString CloudTableModel::categoryName(int category) const {
    if (category >= 0 && category < category_names_.size())
        return category_names_.at(category);
    return QString::number(category);
}
