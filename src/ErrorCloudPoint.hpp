#pragma once
#include "AbstractCloudPoint.hpp"

class ErrorCloudPoint : public AbstractCloudPoint {
public:
    ErrorCloudPoint();
    ErrorCloudPoint(qreal _x, qreal _y, qreal _z, qreal _error);
    ~ErrorCloudPoint();

    QString to_csv() override;

private:
    qreal error_;
};
