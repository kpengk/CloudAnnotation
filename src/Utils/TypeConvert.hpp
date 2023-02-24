#pragma once
#include <QVector>
#include <vector>

template <typename T> QVector<T> from_std_vector(const std::vector<T>& data) {
    return QVector<T>(data.cbegin(), data.cend());
}

template <typename T> std::vector<T> to_std_vector(const QVector<T>& data) {
    return std::vector<T>(data.constBegin(), data.constEnd());
}
