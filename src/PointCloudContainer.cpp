#include "PointCloudContainer.hpp"

PointCloudContainer::PointCloudContainer() {}

PointCloudContainer::~PointCloudContainer() { clear(); }

void PointCloudContainer::append(const DataType& value) { data_.append(value); }

void PointCloudContainer::push_back(const DataType& value) { data_.push_back(value); }

const PointCloudContainer::DataType& PointCloudContainer::at(qsizetype i) const { return data_.at(i); }

void PointCloudContainer::reserve(qsizetype size) { data_.reserve(size); }

void PointCloudContainer::clear() {
    for (auto d : data_) {
        delete d;
    }
    data_.clear();
}

void PointCloudContainer::setHeader(const QStringList& names) { names_ = names; }

const QStringList& PointCloudContainer::header() const { return names_; }
