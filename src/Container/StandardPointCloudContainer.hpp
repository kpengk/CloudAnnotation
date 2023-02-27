#pragma once
#include "AbstractPointCloudContainer.hpp"

class StandardPointCloudContainer: public AbstractPointCloudContainer {
public:
    StandardPointCloudContainer();
    ~StandardPointCloudContainer();

    int pointCount() const override;
    Point3D pointAt(int id) const override;
    Color colorAt(int id) const override;

    QStringList header() const override;
    QString toCSV(int id) const override;

    void reserve(qsizetype size) override;
    void clear() override;
    void append(const Point3D& pt, const Color& color);

protected:
    struct PointData {
        Point3D pt;
        Color color;
    };
    QVector<PointData> datas_;
};
