#include "StandardPointCloudContainer.hpp"

StandardPointCloudContainer::StandardPointCloudContainer() {}

StandardPointCloudContainer::~StandardPointCloudContainer() {}

int StandardPointCloudContainer::pointCount() const { return datas_.size(); }

Point3D StandardPointCloudContainer::pointAt(int id) const {
    if (id < 0 || id >= datas_.size())
        return Point3D{};
    return datas_.at(id).pt;
}

Color StandardPointCloudContainer::colorAt(int id) const {
    if (id < 0 || id >= datas_.size())
        return Color{};
    return datas_.at(id).color;
}

QStringList StandardPointCloudContainer::header() const { return QStringList{"x", "y", "z", "r", "g", "b"}; }

QString StandardPointCloudContainer::toCSV(int id) const {
    if (id < 0 || id >= datas_.size())
        return QString{};

    return QString("%1,%2").arg(datas_.at(id).pt.toCSV()).arg(datas_.at(id).color.toCSV());
}

void StandardPointCloudContainer::reserve(qsizetype size) { datas_.reserve(size); }

void StandardPointCloudContainer::clear() { datas_.clear(); }

void StandardPointCloudContainer::append(const Point3D& pt, const Color& color) { datas_.append(PointData{pt, color}); }
