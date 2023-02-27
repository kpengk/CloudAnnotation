#pragma once
#include "AbstractPointCloudContainer.hpp"

class ErrorPointCloudContainer : public AbstractPointCloudContainer {
public:
    ErrorPointCloudContainer();
    ~ErrorPointCloudContainer();

    int pointCount() const override;
    Point3D pointAt(int id) const override;
    Color colorAt(int id) const override;

    QStringList header() const override;
    QString toCSV(int id) const override;

    void reserve(qsizetype size) override;
    void clear() override;
    void append(const Point3D& pt, qreal dis);

private:
    struct PointData {
        Point3D pt;
        qreal dis;
    };
    QVector<PointData> datas_;
};
