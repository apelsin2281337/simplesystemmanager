#include "../include/logger.hpp"

#include <iostream>
#include <chrono>
#include <format>
#include <fstream>
#include <filesystem>

Logger::Logger() {
    auto now = std::chrono::system_clock::now();
    auto now_local = std::chrono::current_zone()->to_local(now);

    std::string timeString = std::format("{:%d-%m-%Y}", now_local);
    fileName_ = std::format("/var/log/systemmanager/Log_{}.txt", timeString);

    std::filesystem::create_directory("/var/log/systemmanager/");
}

void Logger::log(Status status, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex_);
    try {
        auto now = std::chrono::system_clock::now();
        auto now_local = std::chrono::current_zone()->to_local(now);
        std::string logTime = std::format("{:%d-%m-%Y_%H-%M-%S}", now_local);
        std::ofstream file(fileName_, std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Failed to open log file: " << fileName_ << std::endl;
            return;
        }

        switch (status) {
        case Status::Log:
            file << std::format("[LOG] {}: {}\n", logTime, message);
            break;
        case Status::Error:
            file << std::format("[ERROR] {}: {}\n", logTime, message);
            break;
        case Status::Fatal:
            file << std::format("[FATAL] {}: {}\n", logTime, message);
            break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Logging failed: " << e.what() << std::endl;
    }
}
