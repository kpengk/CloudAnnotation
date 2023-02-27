#include "ErrorCloudPoint.hpp"
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

ErrorCloudPoint::ErrorCloudPoint()
    : AbstractCloudPoint{}
    , error_{} {}

ErrorCloudPoint::ErrorCloudPoint(qreal _x, qreal _y, qreal _z, qreal _error)
    : AbstractCloudPoint(_x, _y, _z)
    , error_{_error} {
    const auto color = map_color(error_);
    setColor(Color(color[0] * 255.0F, color[1] * 255.0F, color[2] * 255.0F));
}

ErrorCloudPoint::~ErrorCloudPoint() {}

QString ErrorCloudPoint::to_csv() { return QString("%1,%2,%3,%4").arg(x_).arg(y_).arg(z_).arg(error_); }
