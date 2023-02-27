#pragma once
#include <QString>
#include <QtGlobal>

struct Point3D {
public:
    Point3D()
        : x_{}
        , y_{}
        , z_{} {}

    Point3D(qreal _x, qreal _y, qreal _z)
        : x_{_x}
        , y_{_y}
        , z_{_z} {}

    qreal x() const { return x_; }
    qreal y() const { return y_; }
    qreal z() const { return z_; }

    QString toCSV() const { return QString("%1,%2,%3").arg(x_).arg(y_).arg(z_); }

private:
    qreal x_{};
    qreal y_{};
    qreal z_{};
};
