// في ملف الرأس المناسب (مثلاً include/memory/rammonitor.h)
#include <Windows.h>
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")

class RamMonitor {
public:
    double getTotalRAM();
    double getUsedRAM();
    double getAvailableRAM();
    // ... المزيد من الوظائف
};

// في ملف التنفيذ (memory/rammonitor.cpp)
double RamMonitor::getTotalRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return static_cast<double>(memInfo.ullTotalPhys) / (1024 * 1024 * 1024); // تحويل إلى GB
}

double RamMonitor::getUsedRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024 * 1024); // تحويل إلى GB
}

double RamMonitor::getAvailableRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return static_cast<double>(memInfo.ullAvailPhys) / (1024 * 1024 * 1024); // تحويل إلى GB
}