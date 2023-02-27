#include "ErrorPointCloudContainer.hpp"
#include <array>

namespace {
double interpolate(qreal value, qreal y0, qreal x0, qreal y1, qreal x1) {
    if (value < x0)
        return y0;
    if (value > x1)
        return y1;
    return (value - x0) * (y1 - y0) / (x1 - x0) + y0;
}

double jet_base(qreal value) {
    if (value <= -0.75)
        return 0.0;
    else if (value <= -0.25)
        return interpolate(value, 0.0, -0.75, 1.0, -0.25);
    else if (value <= 0.25)
        return 1.0;
    else if (value <= 0.75)
        return interpolate(value, 1.0, 0.25, 0.0, 0.75);
    else
        return 0.0;
}

// value: (-0.125, 1.125]
std::array<qreal, 3> map_color(qreal value) {
    return std::array{
        jet_base(value * 2.0 - 1.5), // red
        jet_base(value * 2.0 - 1.0), // green
        jet_base(value * 2.0 - 0.5)  // blue
    };
}
} // namespace

ErrorPointCloudContainer::ErrorPointCloudContainer() {}

ErrorPointCloudContainer::~ErrorPointCloudContainer() {}

int ErrorPointCloudContainer::pointCount() const { return datas_.size(); }

Point3D ErrorPointCloudContainer::pointAt(int id) const {
    if (id < 0 || id >= datas_.size())
        return Point3D{};
    return datas_.at(id).pt;
}

Color ErrorPointCloudContainer::colorAt(int id) const {
    if (id < 0 || id >= datas_.size())
        return Color{};
    const auto color = map_color(datas_.at(id).dis);
    return Color(color[0] * 255.0F, color[1] * 255.0F, color[2] * 255.0F);
}

QStringList ErrorPointCloudContainer::header() const { return QStringList{"x", "y", "z", "error"}; }

QString ErrorPointCloudContainer::toCSV(int id) const {
    if (id < 0 || id >= datas_.size())
        return QString{};

    return QString("%1,%2").arg(datas_.at(id).pt.toCSV()).arg(datas_.at(id).dis);
}

void ErrorPointCloudContainer::reserve(qsizetype size) { datas_.reserve(size); }

void ErrorPointCloudContainer::clear() { datas_.clear(); }

void ErrorPointCloudContainer::append(const Point3D& pt, qreal dis) { datas_.append(PointData{pt, dis}); }
