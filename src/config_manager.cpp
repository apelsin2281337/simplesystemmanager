#include "../include/config_manager.hpp"
#include "../include/json.hpp"
#include "../include/logger.hpp"


#include <iostream>
#include <fstream>
#include <string>

bool Config::load() {
    try {
        std::ifstream file(filename_);
        if (!file.is_open()) {
            logE(std::format("Config: Cannot open file {}", filename_));
            return false;
        }

        file >> data_;
        file.close();

        theme_ = data_.value("theme", "dark");
        language_ = data_.value("language", "en_US");

        return true;
    } catch (const std::exception& e) {
        logE(std::format("Config: Failed loading config {}. Reason: {}.", filename_, e.what()));
        data_ = nlohmann::json::object();
        return false;
    }
}

bool Config::save() {
    try {
        data_["theme"] = theme_;
        data_["language"] = language_;

        std::ofstream file(filename_);
        if (!file.is_open()) {
            logE(std::format("Config: Cannot create file {}", filename_));
            return false;
        }

        file << data_.dump(4);
        file.close();
        return true;
    } catch (const std::exception& e) {
        logE(std::format("Config: Failed saving config {}. Reason: {}.", filename_, e.what()));
        return false;
    }
}

void Config::clear() {
    data_.clear();
    theme_ = "dark";
    language_ = "en_US";
}
