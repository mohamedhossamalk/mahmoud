#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <mutex>
#include <filesystem>

class Logger {
public:
    static void initialize(const std::string& logFilePath = "SystemMonitor.log");
    static void log(const std::string& message);
    static void setLogLevel(int level);

private:
    static std::ofstream logFile;
    static std::mutex logMutex;
    static int logLevel;
    static bool initialized;

    static std::string getCurrentTimestamp();
};

#endif // LOGGER_H