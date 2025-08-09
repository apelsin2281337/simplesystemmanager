#include "include/resource_monitor.hpp"

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

Resmon::NetworkStats Resmon::get_internet_usage(const std::string& interface){
    std::lock_guard<std::mutex> lock(statsMutex_);
    NetworkStats stats;
    std::ifstream file("/proc/net/dev");
    unsigned long long tx_bytes, rx_bytes;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(interface) != std::string::npos) {
            size_t colon_pos = line.find(":");
            line = line.substr(colon_pos + 1);
            sscanf(line.c_str(), "%llu %*u %*u %*u %*u %*u %*u %*u %llu", &rx_bytes, &tx_bytes);
            break;
        }
    }
    file.close();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    file.open("/proc/net/dev");
    unsigned long long new_rx_bytes = 0, new_tx_bytes = 0;
    while (std::getline(file, line)) {
        if (line.find(interface) != std::string::npos) {
            size_t colon_pos = line.find(":");
            line = line.substr(colon_pos + 1);
            sscanf(line.c_str(), "%llu %*u %*u %*u %*u %*u %*u %*u %llu", &new_rx_bytes, &new_tx_bytes);
            break;
        }
    }

    stats.rx_speed = (new_rx_bytes - rx_bytes) / 1024.0;
    stats.tx_speed = (new_tx_bytes - tx_bytes) / 1024.0;
    return stats;
}
