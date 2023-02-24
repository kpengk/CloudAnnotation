#pragma once

#include <nlohmann/json.hpp>

#include <QString>
#include <array>
#include <string>
#include <vector>

class WidgetSettings {
public:
    bool mainwindow_maximized{false};
    std::array<int, 2> mainwindow_size{1366, 768}; // width height
    std::vector<int> mainwindow_splitter_sizes{357, 986};

    bool operator==(const WidgetSettings&) const = default;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WidgetSettings, mainwindow_maximized, mainwindow_size, mainwindow_splitter_sizes);
};

class UserSettings {
public:
    WidgetSettings widget;

    bool operator==(const UserSettings&) const = default;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserSettings, widget);
};
