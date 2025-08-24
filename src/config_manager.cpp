#include "../include/config_manager.hpp"

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
}
