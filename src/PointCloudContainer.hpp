#pragma once
#include "AbstractCloudPoint.hpp"
#include <QVector>
#include <QStringList>

class PointCloudContainer {
public:
    using DataType = AbstractCloudPoint*;
    using const_iterator = QVector<DataType>::const_iterator;
    using iterator = QVector<DataType>::iterator;

    PointCloudContainer();
    ~PointCloudContainer();

    int size() const { return data_.size(); }
    bool isEmpty() const { return size() == 0; }

    void append(const DataType& value);
    void push_back(const DataType& value);
    const DataType& at(qsizetype i) const;

    void reserve(qsizetype size);

    void clear();

    void setHeader(const QStringList& names);
    const QStringList& header() const;

private:
    QVector<AbstractCloudPoint*> data_;
    QStringList names_;
};
