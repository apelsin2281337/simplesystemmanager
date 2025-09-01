#pragma once
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <format>
#include <string>

class TaskManager {
public:
    struct Process {
        std::string name;
        double cpu_usage;
        size_t memory_rss_kb;
        size_t memory_vsize_kb;
        pid_t pid;
    };

    static std::vector<Process> getAllProcessesInfo() {
        std::vector<Process> processes;
        auto pids = get_Pids();

        for (auto pid : pids) {
            try {
                processes.push_back(getProcessInfo(pid));
            } catch (const std::exception& e) {
                logE("Failed to get info for PID " + std::to_string(pid) + ": " + e.what());
            }
        }

        return processes;
    }

    static Process getProcessInfo(pid_t pid) {
        Process process;
        process.pid = pid;

        auto processInfo = parseStatusFile(pid);

        if (!processInfo.empty()) {
            if (processInfo.find("Name") != processInfo.end()) {
                process.name = processInfo["Name"];
            }

            if (processInfo.find("VmRSS") != processInfo.end()) {
                std::string rss_str = processInfo["VmRSS"];
                rss_str = rss_str.substr(0, rss_str.find(" kB"));
                try {
                    process.memory_rss_kb = std::stoul(rss_str);
                } catch (...) {
                    process.memory_rss_kb = 0;
                }
            }

            if (processInfo.find("VmSize") != processInfo.end()) {
                std::string vsize_str = processInfo["VmSize"];
                vsize_str = vsize_str.substr(0, vsize_str.find(" kB"));
                try {
                    process.memory_vsize_kb = std::stoul(vsize_str);
                } catch (...) {
                    process.memory_vsize_kb = 0;
                }
            }

            // CPU usage calculation is more complex and requires sampling over time
            process.cpu_usage = 0.0; // Placeholder - needs proper implementation
        }

        return process;
    }



private:
    static std::unordered_map<std::string, std::string> parseStatusFile(pid_t pid) {
        std::unordered_map<std::string, std::string> status;
        std::ifstream file("/proc/" + std::to_string(pid) + "/status");

        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    status[key] = value;
                }
            }
        } else {
            logE("Failed to open status file for PID: " + std::to_string(pid));
        }
        return status;
    }

    static std::vector<pid_t> get_Pids() {
        std::vector<pid_t> pids;
        try {
            for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
                std::string filename = entry.path().filename().string();
                if (entry.is_directory() && std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                    try {
                        pid_t pid = static_cast<pid_t>(std::stoi(filename));
                        pids.push_back(pid);
                    } catch (const std::exception& e) {
                        logE("Failed to convert PID: " + filename + " - " + e.what());
                    }
                }
            }
            logL(std::format("Task Manager: Found {} processes", pids.size()));
        } catch (const std::filesystem::filesystem_error& e) {
            logE(std::format("Filesystem error in /proc: {}", e.what()));
        } catch (const std::exception& e) {
            logE(std::format("Task Manager: Error occurred: {}", e.what()));
        }
        return pids;
    }





};
