#pragma once
#include <string>
#include "../include/json.hpp"



class Config {
private:
    std::string filename_;
    nlohmann::json data_;
    std::string theme_;
    std::string language_;

public:
    Config(const std::string& configFile = "config.json")
        : filename_(configFile), theme_("dark"), language_("en_US") {
        load();
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    bool load();

    bool save();

    const std::string& getTheme() const { return theme_; }
    const std::string& getLanguage() const { return language_; }

    void setTheme(const std::string& theme) { theme_ = theme; }
    void setLanguage(const std::string& language) { language_ = language; }

    bool has(const std::string& key) const {
        return data_.contains(key);
    }

    void clear();

    const nlohmann::json& getData() const { return data_; }
};
