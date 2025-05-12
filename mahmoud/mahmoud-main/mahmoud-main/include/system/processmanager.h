#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <vector>
#include <string>

// معلومات العملية
struct ProcessInfo {
    std::string name;
    DWORD id;
    SIZE_T memoryUsage;    // بالبايت
    double cpuUsage;       // نسبة مئوية
    std::string status;    // حالة العملية (قيد التشغيل، متوقفة، إلخ)
};

// فئات الأولوية
enum class PriorityClass {
    Idle = IDLE_PRIORITY_CLASS,
    BelowNormal = BELOW_NORMAL_PRIORITY_CLASS,
    Normal = NORMAL_PRIORITY_CLASS,
    AboveNormal = ABOVE_NORMAL_PRIORITY_CLASS,
    High = HIGH_PRIORITY_CLASS,
    RealTime = REALTIME_PRIORITY_CLASS
};

class ProcessManager {
public:
    ProcessManager();
    ~ProcessManager();

    // الحصول على قائمة العمليات
    std::vector<ProcessInfo> getProcessList();

    // الحصول على معلومات عملية محددة
    ProcessInfo getProcessInfo(DWORD processId);

    // إدارة العمليات
    bool setProcessPriority(DWORD processId, PriorityClass priorityClass);
    bool terminateProcess(DWORD processId);

    // مراقبة استخدام الموارد لعملية محددة
    double getProcessCPUUsage(DWORD processId);
    SIZE_T getProcessMemoryUsage(DWORD processId);

private:
    // متغيرات لمراقبة استخدام المعالج للعمليات
    struct ProcessCPUInfo {
        DWORD processId;
        ULONGLONG lastKernelTime;
        ULONGLONG lastUserTime;
        ULONGLONG lastUpdateTime;
    };

    std::vector<ProcessCPUInfo> processCPUInfo;

    // البحث عن معلومات المعالج أو إنشاؤها إذا لم تكن موجودة
    ProcessCPUInfo* findOrCreateProcessCPUInfo(DWORD processId);
};

#endif // PROCESSMANAGER_H
