#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include <Windows.h>
#include <string>

class CacheManager {
public:
    CacheManager();
    ~CacheManager();

    // تنظيف الكاش
    bool flushDiskCache();        // تنظيف كاش القرص
    bool flushDNSCache();         // تنظيف كاش DNS
    bool clearFileSystemCache();  // تنظيف كاش نظام الملفات
    bool clearBrowserCache(const std::string& browser = ""); // تنظيف كاش المتصفح

    // الحصول على معلومات الكاش
    size_t getDiskCacheSize();    // حجم كاش القرص

private:
    bool executeCommand(const std::string& command);
};

#endif // CACHEMANAGER_H