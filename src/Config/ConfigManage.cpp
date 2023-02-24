#include "ConfigManage.hpp"
#include "Utils/FileIO.hpp"
#include <QDir>
#include <QCoreApplication>

ConfigManage* ConfigManage::instance() {
    static ConfigManage obj;
    return &obj;
}

void ConfigManage::load() {
    const auto category_names_opt = read_json<CategoryNames>("config/category_names.json");
    category_names_ = category_names_opt ? *category_names_opt : CategoryNames{};

    const auto user_setting_opt = read_json<UserSettings>("config/user_settings.json");
    user_setting_ = user_setting_opt ? *user_setting_opt : UserSettings{};
}

void ConfigManage::reload() { load(); }

bool ConfigManage::save() {
    /*QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("config")) {
        dir.mkdir("config");
    }*/

    bool flag = write_json("config/user_settings.json", user_setting_);
    return flag;
}

void ConfigManage::set_auto_save(bool enable) { auto_save_ = enable; }

ConfigManage::ConfigManage()
    : auto_save_{true} {
    load();
}

ConfigManage::~ConfigManage() {
    if (auto_save_)
        save();
}
