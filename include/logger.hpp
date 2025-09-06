#pragma once
#include <string>
#include <mutex>


#define Slog(a, b) Logger::getInstance().log(a, b);
#define logL(b) Logger::getInstance().log(Status::Log, b);
#define logE(b) Logger::getInstance().log(Status::Error, b);
#define logF(b) Logger::getInstance().log(Status::Fatal, b);

enum class Status{
    Log, Error, Fatal
};

class Logger{
public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    void log(Status status, const std::string& message);

private:
    Logger();
    std::string fileName_;
    std::string timeString_;
    std::mutex logMutex_;
};
