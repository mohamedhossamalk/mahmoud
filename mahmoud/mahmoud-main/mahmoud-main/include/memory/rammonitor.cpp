#include "../include/memory/rammonitor.h"
#include "../include/utils/logger.h"
#include <chrono>

RamMonitor::RamMonitor() : maxHistorySize(60) { // حفظ 60 سجل كحد أقصى (مثلاً: آخر 60 ثانية)
}

RamMonitor::~RamMonitor() {
}

RAMInfo RamMonitor::getRAMInfo() {
    RAMInfo info = { 0 };

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memInfo)) {
        // الذاكرة الفيزيائية (RAM)
        info.totalPhysical = static_cast<double>(memInfo.ullTotalPhys) / (1024 * 1024 * 1024); // تحويل إلى GB
        info.availablePhysical = static_cast<double>(memInfo.ullAvailPhys) / (1024 * 1024 * 1024);
        info.usedPhysical = info.totalPhysical - info.availablePhysical;

        // الذاكرة الافتراضية (Virtual Memory)
        info.totalVirtual = static_cast<double>(memInfo.ullTotalVirtual) / (1024 * 1024 * 1024);
        info.availableVirtual = static_cast<double>(memInfo.ullAvailVirtual) / (1024 * 1024 * 1024);
        info.usedVirtual = info.totalVirtual - info.availableVirtual;

        // نسبة استخدام الذاكرة
        info.memoryLoad = static_cast<double>(memInfo.dwMemoryLoad);
    }
    else {
        Logger::log("GlobalMemoryStatusEx failed");
    }

    return info;
}

double RamMonitor::getTotalRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<double>(memInfo.ullTotalPhys) / (1024 * 1024 * 1024); // تحويل إلى GB
    }

    return 0.0;
}

double RamMonitor::getUsedRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memInfo)) {
        double totalRAM = static_cast<double>(memInfo.ullTotalPhys) / (1024 * 1024 * 1024);
        double availableRAM = static_cast<double>(memInfo.ullAvailPhys) / (1024 * 1024 * 1024);
        return totalRAM - availableRAM;
    }

    return 0.0;
}

double RamMonitor::getAvailableRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<double>(memInfo.ullAvailPhys) / (1024 * 1024 * 1024); // تحويل إلى GB
    }

    return 0.0;
}

bool RamMonitor::optimizeRAM() {
    bool success = false;

    // الحصول على قائمة العمليات واستدعاء EmptyWorkingSet لكل عملية
    DWORD processes[1024], cbNeeded;
    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        int numProcesses = cbNeeded / sizeof(DWORD);
        int optimizedProcesses = 0;

        for (int i = 0; i < numProcesses; i++) {
            DWORD processID = processes[i];
            if (processID != 0) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_QUOTA, FALSE, processID);
                if (hProcess != NULL) {
                    if (EmptyWorkingSet(hProcess)) {
                        optimizedProcesses++;
                    }
                    CloseHandle(hProcess);
                }
            }
        }

        success = (optimizedProcesses > 0);

        // تسجيل عدد العمليات التي تم تحسينها
        Logger::log("Optimized RAM for " + std::to_string(optimizedProcesses) + " processes");
    }
    else {
        Logger::log("EnumProcesses failed during RAM optimization");
    }

    return success;
}

std::vector<RAMUsageRecord> RamMonitor::getRAMUsageHistory() {
    return ramUsageHistory;
}

void RamMonitor::updateRAMUsageHistory() {
    // الحصول على الزمن الحالي
    auto currentTime = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime.time_since_epoch()
    ).count();

    // الحصول على استخدام الذاكرة الحالي
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    // إنشاء سجل جديد
    RAMUsageRecord record;
    record.timestamp = timestamp;
    record.memoryUsage = static_cast<double>(memInfo.dwMemoryLoad);

    // إضافة السجل إلى التاريخ
    ramUsageHistory.push_back(record);

    // التأكد من عدم تجاوز الحد الأقصى
    if (ramUsageHistory.size() > maxHistorySize) {
        ramUsageHistory.erase(ramUsageHistory.begin());
    }
}