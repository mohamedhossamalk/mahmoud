#include "../include/memory/cachemanager.h"
#include "../include/utils/logger.h"
#include <filesystem>
#include <ShlObj.h>
#include <KnownFolders.h>

CacheManager::CacheManager() {
}

CacheManager::~CacheManager() {
}

bool CacheManager::flushDiskCache() {
    // تنظيف كاش القرص في Windows باستخدام أوامر النظام
    Logger::log("Flushing disk cache...");

    // استخدام أمر لإجبار كتابة البيانات المخزنة مؤقتًا إلى القرص
    bool result = executeCommand("powershell -Command \"Write-VolumeCache -DriveLetter C\"");

    // يمكن أيضًا تنظيف كاش نظام الملفات
    clearFileSystemCache();

    return result;
}

bool CacheManager::flushDNSCache() {
    // تنظيف كاش DNS في Windows
    Logger::log("Flushing DNS cache...");
    return executeCommand("ipconfig /flushdns");
}

bool CacheManager::clearFileSystemCache() {
    // تنظيف كاش نظام الملفات
    Logger::log("Clearing file system cache...");

    // هذا الأمر يتطلب امتيازات المسؤول
    return executeCommand("powershell -Command \"& {Clear-FileSystemCache}\"");
}

bool CacheManager::clearBrowserCache(const std::string& browser) {
    Logger::log("Clearing browser cache for: " + (browser.empty() ? "all browsers" : browser));

    // الحصول على مجلد AppData
    wchar_t* appDataPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataPath))) {
        std::filesystem::path basePath(appDataPath);
        CoTaskMemFree(appDataPath);

        if (browser.empty() || browser == "chrome") {
            // تنظيف كاش Chrome
            std::filesystem::path chromeCachePath = basePath / "Google" / "Chrome" / "User Data" / "Default" / "Cache";
            if (std::filesystem::exists(chromeCachePath)) {
                try {
                    for (const auto& entry : std::filesystem::directory_iterator(chromeCachePath)) {
                        try {
                            std::filesystem::remove_all(entry.path());
                        }
                        catch (const std::exception& e) {
                            Logger::log("Failed to remove Chrome cache entry: " + std::string(e.what()));
                        }
                    }
                    Logger::log("Chrome cache cleared successfully");
                }
                catch (const std::exception& e) {
                    Logger::log("Failed to clear Chrome cache: " + std::string(e.what()));
                    return false;
                }
            }
        }

        if (browser.empty() || browser == "firefox") {
            // تنظيف كاش Firefox
            std::filesystem::path firefoxPath = basePath / "Mozilla" / "Firefox" / "Profiles";
            if (std::filesystem::exists(firefoxPath)) {
                try {
                    for (const auto& profile : std::filesystem::directory_iterator(firefoxPath)) {
                        std::filesystem::path cachePath = profile.path() / "cache2";
                        if (std::filesystem::exists(cachePath)) {
                            for (const auto& entry : std::filesystem::directory_iterator(cachePath)) {
                                try {
                                    std::filesystem::remove_all(entry.path());
                                }
                                catch (const std::exception& e) {
                                    Logger::log("Failed to remove Firefox cache entry: " + std::string(e.what()));
                                }
                            }
                        }
                    }
                    Logger::log("Firefox cache cleared successfully");
                }
                catch (const std::exception& e) {
                    Logger::log("Failed to clear Firefox cache: " + std::string(e.what()));
                    return false;
                }
            }
        }

        if (browser.empty() || browser == "edge") {
            // تنظيف كاش Edge
            std::filesystem::path edgeCachePath = basePath / "Microsoft" / "Edge" / "User Data" / "Default" / "Cache";
            if (std::filesystem::exists(edgeCachePath)) {
                try {
                    for (const auto& entry : std::filesystem::directory_iterator(edgeCachePath)) {
                        try {
                            std::filesystem::remove_all(entry.path());
                        }
                        catch (const std::exception& e) {
                            Logger::log("Failed to remove Edge cache entry: " + std::string(e.what()));
                        }
                    }
                    Logger::log("Edge cache cleared successfully");
                }
                catch (const std::exception& e) {
                    Logger::log("Failed to clear Edge cache: " + std::string(e.what()));
                    return false;
                }
            }
        }

        return true;
    }
    else {
        Logger::log("Failed to get AppData folder path");
        return false;
    }
}

size_t CacheManager::getDiskCacheSize() {
    // هذه الوظيفة تقدر حجم كاش القرص
    // ملاحظة: هذه تقديرية وليست دقيقة تماماً لأن Windows لا يوفر واجهة برمجة مباشرة للحصول على حجم كاش القرص

    size_t totalCacheSize = 0;

    // الحصول على مجلد AppData
    wchar_t* appDataPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataPath))) {
        std::filesystem::path basePath(appDataPath);
        CoTaskMemFree(appDataPath);

        // حساب حجم كاش المتصفحات الشائعة
        std::vector<std::filesystem::path> cachePaths = {
            basePath / "Google" / "Chrome" / "User Data" / "Default" / "Cache",
            basePath / "Microsoft" / "Edge" / "User Data" / "Default" / "Cache",
            basePath / "Mozilla" / "Firefox" / "Profiles"
        };

        for (const auto& cachePath : cachePaths) {
            if (std::filesystem::exists(cachePath)) {
                try {
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(cachePath)) {
                        if (std::filesystem::is_regular_file(entry)) {
                            totalCacheSize += std::filesystem::file_size(entry);
                        }
                    }
                }
                catch (const std::exception& e) {
                    Logger::log("Error calculating cache size: " + std::string(e.what()));
                }
            }
        }
    }

    return totalCacheSize;
}

bool CacheManager::executeCommand(const std::string& command) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // تحويل الأمر إلى نسخة غير ثابتة
    char* cmd = _strdup(command.c_str());

    // تنفيذ الأمر
    bool result = CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (result) {
        // انتظار انتهاء العملية
        WaitForSingleObject(pi.hProcess, INFINITE);

        // الحصول على رمز الخروج
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        // إغلاق مقابض العملية
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        free(cmd);
        return (exitCode == 0);
    }
    else {
        free(cmd);
        Logger::log("Failed to execute command: " + command + ", Error: " + std::to_string(GetLastError()));
        return false;
    }
}