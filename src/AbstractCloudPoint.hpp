#pragma once
#include "Color.hpp"
#include <QString>
#include <QVector>

class AbstractCloudPoint {
public:
    AbstractCloudPoint()
        : x_{}
        , y_{}
        , z_{}
        , color_{} {}

    AbstractCloudPoint(qreal x, qreal y, qreal z, Color color = {})
        : x_{x}
        , y_{y}
        , z_{z}
        , color_{color} {}

    virtual ~AbstractCloudPoint() {}

    qreal x() const { return x_; }
    qreal y() const { return y_; }
    qreal z() const { return z_; }
    Color color() const { return color_; }
    void setColor(Color color) { color_ = color; }

    virtual QString to_csv() {
        return QString("%1,%2,%3,%4,%5,%6")
            .arg(x_)
            .arg(y_)
            .arg(z_)
            .arg(color_.red())
            .arg(color_.gree())
            .arg(color_.blue());
    }

protected:
    qreal x_;
    qreal y_;
    qreal z_;
    Color color_;
};
