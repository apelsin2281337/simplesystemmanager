#include "../include/task_manager.hpp"
#include "../include/logger.hpp"

std::vector<ProcessInfo> TaskManager::getProcessesInfo() {
    std::vector<ProcessInfo> processes;

    FILE* pipe = popen("ps aux", "r");
    if (!pipe) {
        logE("TaskManager: Could not execute command \"ps aux\"");
        return processes;
    }

    char buffer[2048];
    bool isFirstLine = true;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);

        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }

        ProcessInfo proc = parseLine(line);
        if (proc.pid != -1) {
            processes.push_back(proc);
        }
    }

    pclose(pipe);
    return processes;
}


ProcessInfo TaskManager::parseLine(const std::string& line){
    ProcessInfo proc;
    std::istringstream iss(line);
    iss >> proc.user >> proc.pid >> proc.cpuLoad >> proc.memLoad >> proc.vsz >> proc.rss >> proc.tty >> proc.stat >> proc.start >> proc.time;

    std::getline(iss, proc.command);
    if (!proc.command.empty() && proc.command[0] == ' ')
        proc.command.erase(0, 1);

    return proc;
}
