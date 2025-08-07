
#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/statvfs.h>

class Resmon {
private:
    static std::vector<std::string> get_cpu_stats() {
        std::ifstream file("/proc/stat");
        std::string line;
        std::vector<std::string> stats;

        if (file.is_open()) {
            getline(file, line);
            std::istringstream iss(line);
            std::string token;
            while (iss >> token) {
                if (token != "cpu") stats.push_back(token);
            }
        }
        return stats;
    }

    static std::vector<std::string> get_mem_stats() {
        std::ifstream file("/proc/meminfo");
        std::string line;
        std::vector<std::string> stats;

        if (file.is_open()) {
            while (getline(file, line)) {
                std::istringstream iss(line);
                std::string key, value;
                iss >> key >> value;
                if (key == "MemTotal:" || key == "MemFree:" || key == "MemAvailable:" ||
                    key == "SwapTotal:" || key == "SwapFree:") {
                    stats.push_back(value);
                }
            }
        }
        return stats;
    }

public:
    struct CPUStats {
        uint64_t user;
        uint64_t nice;
        uint64_t system;
        uint64_t idle;

        uint64_t total() const { return user + nice + system + idle; }
        double usage_percent(const CPUStats& prev) const {
            const uint64_t total_diff = total() - prev.total();
            if (total_diff == 0) return 0.0;
            return (total_diff - (idle - prev.idle)) * 100.0 / total_diff;
        }
    };

    struct MemStats {
        uint64_t total;
        uint64_t free;
        uint64_t available;
        uint64_t swaptotal;
        uint64_t swapfree;

        double usage_percent() const {
            if (total == 0) return 0.0;
            return (total - available) * 100.0 / total;
        }

        double swap_usage_percent() const{
            if (swaptotal == 0) return 0.0;
            return (swaptotal - swapfree) * 100.0 / swaptotal;
        }
    };

    struct DiskStats {
        uint64_t total;
        uint64_t free;
        uint64_t used;

        double usage_percent() const {
            if (total == 0) return 0.0;
            return used * 100.0 / total;
        }
    };

    static DiskStats get_disk_usage() {
        DiskStats result = {0, 0, 0};
        struct statvfs vfs;

        if (statvfs("/", &vfs) == 0) {
            result.total = vfs.f_blocks * vfs.f_frsize;
            result.free = vfs.f_bfree * vfs.f_frsize;
            result.used = result.total - result.free;
        }
        return result;
    }


    static CPUStats get_cpu_usage() {
        auto stats = get_cpu_stats();
        CPUStats result;
        if (stats.size() >= 4) {
            result.user = std::stoull(stats[0]);
            result.nice = std::stoull(stats[1]);
            result.system = std::stoull(stats[2]);
            result.idle = std::stoull(stats[3]);
        }
        return result;
    }

    static MemStats get_mem_usage() {
        auto stats = get_mem_stats();
        MemStats result;
        if (stats.size() >= 5) {
            result.total = std::stoull(stats[0]) * 1024;
            result.free = std::stoull(stats[1]) * 1024;
            result.available = std::stoull(stats[2]) * 1024;
            result.swaptotal = std::stoull(stats[3]) * 1024;
            result.swapfree = std::stoull(stats[4]) * 1024;
        }
        return result;
    }
};
