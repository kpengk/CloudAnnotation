#pragma once
#include "Color.hpp"
#include "Point3D.hpp"
#include <QStringList>

class AbstractPointCloudContainer {
public:
    AbstractPointCloudContainer() = default;
    virtual ~AbstractPointCloudContainer() = default;

    virtual int pointCount() const = 0;
    virtual Point3D pointAt(int id) const = 0;
    virtual Color colorAt(int id) const = 0;

    virtual QStringList header() const = 0;
    virtual QString toCSV(int id) const = 0;

    virtual void reserve(qsizetype size) = 0;
    virtual void clear() = 0;
};
