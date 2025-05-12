#include "../include/system/processmanager.h"
#include "../include/utils/logger.h"
#include <sstream>
#include <iomanip>

ProcessManager::ProcessManager() {
}

ProcessManager::~ProcessManager() {
}

std::vector<ProcessInfo> ProcessManager::getProcessList() {
    std::vector<ProcessInfo> processes;

    // التقاط لقطة من العمليات الحالية
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        Logger::log("CreateToolhelp32Snapshot failed");
        return processes;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // الحصول على العملية الأولى
    if (Process32First(hSnapshot, &pe32)) {
        do {
            ProcessInfo process;
            process.name = pe32.szExeFile;
            process.id = pe32.th32ProcessID;

            // الحصول على معلومات الذاكرة
            process.memoryUsage = getProcessMemoryUsage(pe32.th32ProcessID);

            // الحصول على استخدام المعالج
            process.cpuUsage = getProcessCPUUsage(pe32.th32ProcessID);

            // تحديد حالة العملية
            process.status = "Running"; // يمكن توسيعه للحصول على حالة أكثر تفصيلاً

            processes.push_back(process);

        } while (Process32Next(hSnapshot, &pe32));
    }
    else {
        Logger::log("Process32First failed");
    }

    CloseHandle(hSnapshot);
    return processes;
}

ProcessInfo ProcessManager::getProcessInfo(DWORD processId) {
    ProcessInfo process;
    process.id = processId;

    // فتح العملية للوصول إلى المعلومات
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        // الحصول على اسم العملية
        char processName[MAX_PATH] = "<unknown>";
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
            GetModuleBaseNameA(hProcess, hMod, processName, sizeof(processName));
        }

        process.name = processName;

        // الحصول على استخدام الذاكرة
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            process.memoryUsage = pmc.WorkingSetSize;
        }

        // الحصول على استخدام المعالج
        process.cpuUsage = getProcessCPUUsage(processId);

        // تحديد حالة العملية
        process.status = "Running";

        CloseHandle(hProcess);
    }
    else {
        std::ostringstream oss;
        oss << "Failed to open process with ID: " << processId << " - Error: " << GetLastError();
        Logger::log(oss.str());
    }

    return process;
}

bool ProcessManager::setProcessPriority(DWORD processId, PriorityClass priorityClass) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, processId);
    if (!hProcess) {
        std::ostringstream oss;
        oss << "Failed to open process with ID: " << processId << " for setting priority - Error: " << GetLastError();
        Logger::log(oss.str());
        return false;
    }

    bool result = SetPriorityClass(hProcess, static_cast<DWORD>(priorityClass));

    if (!result) {
        std::ostringstream oss;
        oss << "Failed to set priority for process with ID: " << processId << " - Error: " << GetLastError();
        Logger::log(oss.str());
    }

    CloseHandle(hProcess);
    return result;
}

bool ProcessManager::terminateProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (!hProcess) {
        std::ostringstream oss;
        oss << "Failed to open process with ID: " << processId << " for termination - Error: " << GetLastError();
        Logger::log(oss.str());
        return false;
    }

    bool result = TerminateProcess(hProcess, 0);

    if (!result) {
        std::ostringstream oss;
        oss << "Failed to terminate process with ID: " << processId << " - Error: " << GetLastError();
        Logger::log(oss.str());
    }

    CloseHandle(hProcess);
    return result;
}

SIZE_T ProcessManager::getProcessMemoryUsage(DWORD processId) {
    SIZE_T memoryUsage = 0;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            memoryUsage = pmc.WorkingSetSize;
        }
        CloseHandle(hProcess);
    }

    return memoryUsage;
}

double ProcessManager::getProcessCPUUsage(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) {
        return 0.0;
    }

    FILETIME createTime, exitTime, kernelTime, userTime;
    if (!GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
        CloseHandle(hProcess);
        return 0.0;
    }

    ULARGE_INTEGER kernelTimeValue, userTimeValue, currentTime;
    kernelTimeValue.LowPart = kernelTime.dwLowDateTime;
    kernelTimeValue.HighPart = kernelTime.dwHighDateTime;
    userTimeValue.LowPart = userTime.dwLowDateTime;
    userTimeValue.HighPart = userTime.dwHighDateTime;

    FILETIME systemTime;
    GetSystemTimeAsFileTime(&systemTime);
    currentTime.LowPart = systemTime.dwLowDateTime;
    currentTime.HighPart = systemTime.dwHighDateTime;

    ProcessCPUInfo* info = findOrCreateProcessCPUInfo(processId);

    if (info->lastUpdateTime == 0) {
        // الاستدعاء الأول، تخزين القيم الأولية
        info->lastKernelTime = kernelTimeValue.QuadPart;
        info->lastUserTime = userTimeValue.QuadPart;
        info->lastUpdateTime = currentTime.QuadPart;
        CloseHandle(hProcess);
        return 0.0;
    }

    // حساب الوقت المنقضي
    ULONGLONG kernelDiff = kernelTimeValue.QuadPart - info->lastKernelTime;
    ULONGLONG userDiff = userTimeValue.QuadPart - info->lastUserTime;
    ULONGLONG timeDiff = currentTime.QuadPart - info->lastUpdateTime;

    // تحديث القيم
    info->lastKernelTime = kernelTimeValue.QuadPart;
    info->lastUserTime = userTimeValue.QuadPart;
    info->lastUpdateTime = currentTime.QuadPart;

    CloseHandle(hProcess);

    if (timeDiff == 0) {
        return 0.0;
    }

    // حساب النسبة المئوية (نسبة الوقت المستهلك / إجمالي الوقت المنقضي * عدد المعالجات)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    double cpuUsage = ((kernelDiff + userDiff) * 100.0) / (timeDiff * sysInfo.dwNumberOfProcessors);
    return (cpuUsage < 0.0 || cpuUsage > 100.0) ? 0.0 : cpuUsage;
}

ProcessManager::ProcessCPUInfo* ProcessManager::findOrCreateProcessCPUInfo(DWORD processId) {
    // البحث عن معلومات العملية الحالية
    for (auto& info : processCPUInfo) {
        if (info.processId == processId) {
            return &info;
        }
    }

    // إنشاء معلومات جديدة إذا لم تكن موجودة
    ProcessCPUInfo newInfo;
    newInfo.processId = processId;
    newInfo.lastKernelTime = 0;
    newInfo.lastUserTime = 0;
    newInfo.lastUpdateTime = 0;

    processCPUInfo.push_back(newInfo);
    return &processCPUInfo.back();
}
