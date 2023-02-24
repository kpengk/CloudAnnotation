#pragma once
#include "Config/UserSettings.hpp"

using CategoryNames = std::vector<std::string>;

class ConfigManage {
public:
    static ConfigManage* instance();
    void load();
    void reload();
    bool save();
    void set_auto_save(bool enable);

    const CategoryNames& category_names() const { return category_names_; }

    UserSettings& user_setting() { return user_setting_; }

private:
    ConfigManage();
    ~ConfigManage();
    ConfigManage(const ConfigManage&) = delete;
    ConfigManage(ConfigManage&&) = delete;
    ConfigManage operator=(const ConfigManage&) = delete;

private:
    bool auto_save_;
    // read only
    CategoryNames category_names_;
    // read write
    UserSettings user_setting_;
};
