#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <mutex>
#include <cstdlib>
#include <stdexcept>
#include <format>


#include "../include/resource_monitor.hpp"

std::mutex Resmon::statsMutex_;

std::vector<std::string> Resmon::get_cpu_stats() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    std::ifstream file("/proc/stat");
    std::string line;
    std::vector<std::string> stats;

    if (file.is_open()) {
        std::getline(file, line);
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            if (token != "cpu") stats.push_back(token);
        }
    } else {
        logE("Failed to open /proc/stat");
    }
    return stats;
}

std::string Resmon::get_cpu_name() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    std::string result = "Unknown CPU";

    while (std::getline(file, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t colon_pos = line.find(":");
            if (colon_pos != std::string::npos) {
                result = line.substr(colon_pos + 1);
                size_t first_non_ws = result.find_first_not_of(" \t");
                if (first_non_ws != std::string::npos) {
                    result = result.substr(first_non_ws);
                }

                size_t last_non_ws = result.find_last_not_of(" \t");
                if (last_non_ws != std::string::npos) {
                    result = result.substr(0, last_non_ws + 1);
                }

                break;
            }
        }
    }

    logL(result);
    return result;
}

std::vector<std::string> Resmon::get_mem_stats() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    std::ifstream file("/proc/meminfo");
    std::string line;
    std::vector<std::string> stats;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key, value;
            iss >> key >> value;
            if (key == "MemTotal:" || key == "MemFree:" || key == "MemAvailable:" ||
                key == "SwapTotal:" || key == "SwapFree:") {
                stats.push_back(value);
            }
        }
    } else {
        logE("Failed to open /proc/meminfo");
    }
    return stats;
}

Resmon::DiskStats Resmon::get_disk_usage() {
    DiskStats result = {0, 0, 0};
    struct statvfs vfs;

    if (statvfs("/", &vfs) == 0) {
        result.total = vfs.f_blocks * vfs.f_frsize;
        result.free = vfs.f_bfree * vfs.f_frsize;
        result.used = result.total - result.free;
        //logL(std::format("Disk stats: total={} bytes, free={} bytes", result.total, result.free));
    } else {
        logE("Failed to get disk stats using statvfs");
    }
    return result;
}

Resmon::CPUStats Resmon::get_cpu_usage() {
    auto stats = get_cpu_stats();
    CPUStats result;
    if (stats.size() >= 4) {
        result.user = std::stoull(stats[0]);
        result.nice = std::stoull(stats[1]);
        result.system = std::stoull(stats[2]);
        result.idle = std::stoull(stats[3]);
        //logL(std::format("CPU stats: user={}, nice={}, system={}, idle={}",
        //                 result.user, result.nice, result.system, result.idle));
    } else {
        logE("Invalid CPU stats format from /proc/stat");
    }
    return result;
}


Resmon::MemStats Resmon::get_mem_usage() {
    auto stats = get_mem_stats();
    MemStats result;
    if (stats.size() >= 5) {
        result.total = std::stoull(stats[0]) * 1024;
        result.free = std::stoull(stats[1]) * 1024;
        result.available = std::stoull(stats[2]) * 1024;
        result.swaptotal = std::stoull(stats[3]) * 1024;
        result.swapfree = std::stoull(stats[4]) * 1024;
        //logL(std::format("Memory stats: total={} bytes, available={} bytes",
        //                result.total, result.available));
    } else {
        logE("Invalid memory stats format from /proc/meminfo");
    }
    return result;
}


std::string Resmon::get_network_interface() {
    std::ifstream file("/proc/net/dev");
    if (!file.is_open()) {
        logE("Failed to open /proc/net/dev");
        return "";
    }

    std::string line;
    std::getline(file, line);
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.find("lo:") != std::string::npos || line.find("docker") != std::string::npos ||
            line.find("virbr") != std::string::npos || line.find("veth") != std::string::npos) {
            continue;
        }

        std::istringstream iss(line);
        std::string interface;
        iss >> interface;

        if (interface.back() == ':') {
            interface.pop_back();
            return interface;
        }
    }

    return "";
}

Resmon::NetworkStats Resmon::get_internet_usage() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    NetworkStats stats = {0, 0};
    static unsigned long long last_rx_bytes = 0, last_tx_bytes = 0;
    unsigned long long rx_bytes = 0, tx_bytes = 0;

    std::ifstream file("/proc/net/dev");
    if (!file.is_open()) {
        logE("Failed to open /proc/net/dev");
        return stats;
    }

    std::string line;
    std::getline(file, line);
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.find("lo:") == 0 || line.find("docker") != std::string::npos ||
            line.find("virbr") != std::string::npos || line.find("veth") != std::string::npos) {
            continue;
        }

        std::istringstream iss(line);
        std::string interface;
        unsigned long long rbytes, tbytes;

        iss >> interface;
        if (interface.back() == ':') {
            interface.pop_back();
            iss >> rbytes;
            for (int i = 0; i < 7; ++i) {
                unsigned long long dummy;
                iss >> dummy;
            }
            iss >> tbytes;

            rx_bytes += rbytes;
            tx_bytes += tbytes;
        }
    }

    if (last_rx_bytes > 0 && last_tx_bytes > 0) {
        stats.rx_speed = (rx_bytes - last_rx_bytes) / 1024;
        stats.tx_speed = (tx_bytes - last_tx_bytes) / 1024;
    }

    last_rx_bytes = rx_bytes;
    last_tx_bytes = tx_bytes;

    return stats;
}
