#pragma once
#include "../include/json.hpp"
#include "../include/logger.hpp"
#include <iostream>
#include <fstream>
#include <string>

class Config {
private:
    std::string filename_;
    nlohmann::json data_;
    std::string theme_;

public:
    Config(const std::string& configFile = "config.json")
        : filename_(configFile), theme_("dark") {
        load();
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    bool load();

    bool save();

    const std::string& getTheme() const { return theme_; }

    void setTheme(const std::string& theme) { theme_ = theme; }

    bool has(const std::string& key) const {
        return data_.contains(key);
    }

    void clear();

    const nlohmann::json& getData() const { return data_; }
};
