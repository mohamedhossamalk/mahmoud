#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <Windows.h>
#include <vector>
#include <string>
#include "../memory/rammonitor.h"
#include "../memory/cachemanager.h"
#include "processmanager.h"

// معلومات موارد النظام
struct SystemResources {
    double cpuUsage;
    double totalRAM;
    double usedRAM;
    double availableRAM;
    double diskUsage;
};

class SystemMonitor {
public:
    SystemMonitor();
    ~SystemMonitor();

    // الحصول على معلومات موارد النظام
    SystemResources getSystemResources();
    double getCPUUsage();

    // إدارة الذاكرة
    RamMonitor& getRamMonitor() { return ramMonitor; }
    CacheManager& getCacheManager() { return cacheManager; }
    ProcessManager& getProcessManager() { return processManager; }

private:
    RamMonitor ramMonitor;
    CacheManager cacheManager;
    ProcessManager processManager;

    // مراقبة استخدام المعالج
    ULARGE_INTEGER lastCPUIdleTime;
    ULARGE_INTEGER lastCPUKernelTime;
    ULARGE_INTEGER lastCPUUserTime;
    bool firstCPUQuery;
};

#endif // SYSTEMMONITOR_H
