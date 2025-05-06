#include "../include/utils/logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

// تهيئة المتغيرات الساكنة
std::ofstream Logger::logFile;
std::mutex Logger::logMutex;
int Logger::logLevel = 0;
bool Logger::initialized = false;

void Logger::initialize(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(logMutex);

    if (initialized) {
        // إذا كان قد تم تهيئة السجل بالفعل، قم بإغلاقه وإعادة فتحه
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    // فتح ملف السجل للإضافة
    logFile.open(logFilePath, std::ios::app);

    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
        return;
    }

    initialized = true;

    // كتابة سجل بدء التطبيق
    logFile << "\n----------------------------------------------------\n";
    logFile << "System Monitor Log Started at " << getCurrentTimestamp() << "\n";
    logFile << "----------------------------------------------------\n";
    logFile.flush();
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);

    if (!initialized) {
        // إذا لم تتم التهيئة بعد، قم بتهيئة السجل مع ملف افتراضي
        initialize();
    }

    // كتابة الرسالة إلى السجل مع الطابع الزمني
    logFile << getCurrentTimestamp() << " - " << message << std::endl;
    logFile.flush();

#ifdef _DEBUG
    // في وضع التصحيح، أيضًا اطبع الرسالة في وحدة التحكم
    std::cout << getCurrentTimestamp() << " - " << message << std::endl;
#endif
}

void Logger::setLogLevel(int level) {
    std::lock_guard<std::mutex> lock(logMutex);
    logLevel = level;
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}