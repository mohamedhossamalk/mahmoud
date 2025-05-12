#ifndef RAMMONITOR_H
#define RAMMONITOR_H

#include <Windows.h>
#include <vector>

// معلومات استخدام الذاكرة العشوائية
struct RAMInfo {
    double totalPhysical;      // بالجيجا بايت
    double availablePhysical;  // بالجيجا بايت
    double usedPhysical;       // بالجيجا بايت

    double totalVirtual;       // بالجيجا بايت
    double availableVirtual;   // بالجيجا بايت
    double usedVirtual;        // بالجيجا بايت

    double memoryLoad;         // نسبة استخدام الذاكرة (0-100)
};

// سجل استخدام الذاكرة (للرسوم البيانية)
struct RAMUsageRecord {
    ULONGLONG timestamp;       // الزمن بالمللي ثانية
    double memoryUsage;        // نسبة الاستخدام (0-100)
};

class RamMonitor {
public:
    RamMonitor();
    ~RamMonitor();

    // الحصول على معلومات الذاكرة
    RAMInfo getRAMInfo();
    double getTotalRAM();
    double getUsedRAM();
    double getAvailableRAM();

    // محاولة تحسين استخدام الذاكرة
    bool optimizeRAM();

    // الحصول على سجل استخدام الذاكرة (للرسوم البيانية)
    std::vector<RAMUsageRecord> getRAMUsageHistory();

    // تحديث سجل الاستخدام
    void updateRAMUsageHistory();

private:
    // سجل بيانات استخدام الذاكرة
    std::vector<RAMUsageRecord> ramUsageHistory;
    int maxHistorySize;
};

#endif // RAMMONITOR_H
