#pragma once


class Resmon {
private:
    static std::mutex statsMutex_;
    static std::vector<std::string> get_cpu_stats();
    static std::vector<std::string> get_mem_stats();

public:
    struct CPUStats {
        uint64_t user = 0;
        uint64_t nice = 0;
        uint64_t system = 0;
        uint64_t idle = 0;

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

    struct NetworkStats {
        size_t rx_speed;
        size_t tx_speed;
    };



    static DiskStats get_disk_usage();

    static NetworkStats get_internet_usage();
    static std::string get_network_interface();

    static CPUStats get_cpu_usage();
    static std::string get_cpu_name();

    static MemStats get_mem_usage();
};
