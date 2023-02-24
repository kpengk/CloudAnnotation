#pragma once

#include <nlohmann/json.hpp>

#include <QDebug>
#include <QString>

#include <optional>
#include <string>

std::string read_file(const QString& filename);
bool write_file(const QString& filename, const std::string& text);

template <typename T> std::optional<T> read_json(const QString& filename) {
    const auto text = read_file(filename);
    if (text.empty()) {
        return std::nullopt;
    }

    try {
        const auto json_obj = nlohmann::json::parse(text);
        const T obj = json_obj.get<T>();
        return obj;
    } catch (std::exception& ex) {
        qCritical() << ex.what();
        return std::nullopt;
    }
}

template <typename T> bool write_json(const QString& filename, const T& obj) {
    try {
        const nlohmann::json json_obj = obj;
        const std::string text = json_obj.dump(4);
        return write_file(filename, text);
    } catch (std::exception& ex) {
        qCritical() << ex.what();
        return false;
    }
}
