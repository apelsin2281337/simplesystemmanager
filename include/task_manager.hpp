#pragma once
#include <string>
#include <sstream>
#include <vector>

struct ProcessInfo{
    std::string user;
    int pid;
    double cpuLoad, memLoad;
    size_t vsz, rss;
    std::string tty, stat, start, time, command;


};

class TaskManager{
private:
    static ProcessInfo parseLine(const std::string& line);

public:
    TaskManager() = default;
    static std::vector<ProcessInfo> getProcessesInfo();

};

