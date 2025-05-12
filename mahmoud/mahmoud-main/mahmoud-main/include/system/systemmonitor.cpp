#include "../include/system/systemmonitor.h"
#include "../include/utils/oshelper.h"
#include <pdh.h>
#include <psapi.h>

#pragma comment(lib, "pdh.lib")

SystemMonitor::SystemMonitor() : firstCPUQuery(true) {
    // تهيئة مراقبة المعالج
    getCPUUsage(); // الاستدعاء الأول يهيئ القيم
}

SystemMonitor::~SystemMonitor() {
}

SystemResources SystemMonitor::getSystemResources() {
    SystemResources resources;

    resources.cpuUsage = getCPUUsage();
    resources.totalRAM = ramMonitor.getTotalRAM();
    resources.usedRAM = ramMonitor.getUsedRAM();
    resources.availableRAM = ramMonitor.getAvailableRAM();

    // يمكن إضافة مزيد من المعلومات لاحقاً

    return resources;
}

double SystemMonitor::getCPUUsage() {
    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return -1.0;
    }

    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idleTime.dwLowDateTime;
    idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;

    if (firstCPUQuery) {
        firstCPUQuery = false;
        lastCPUIdleTime = idle;
        lastCPUKernelTime = kernel;
        lastCPUUserTime = user;
        return 0.0;
    }

    // حساب الوقت المنقضي
    ULONGLONG idleDiff = idle.QuadPart - lastCPUIdleTime.QuadPart;
    ULONGLONG kernelDiff = kernel.QuadPart - lastCPUKernelTime.QuadPart;
    ULONGLONG userDiff = user.QuadPart - lastCPUUserTime.QuadPart;

    // إجمالي وقت المعالج = وقت النواة + وقت المستخدم
    ULONGLONG totalDiff = kernelDiff + userDiff;

    // حساب نسبة استخدام المعالج
    double cpuUsage = 0;
    if (totalDiff > 0) {
        cpuUsage = 100.0 - ((idleDiff * 100.0) / totalDiff);
    }

    // تحديث القيم الأخيرة للحسابات القادمة
    lastCPUIdleTime = idle;
    lastCPUKernelTime = kernel;
    lastCPUUserTime = user;

    return cpuUsage;
}
