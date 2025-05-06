#ifndef OSHELPER_H
#define OSHELPER_H

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>

class OSHelper {
public:
    // الحصول على معلومات نظام التشغيل
    static std::string getOSVersion();
    static std::string getComputerName();
    static std::string getUserName();
    static bool isAdministrator();

    // التحويل بين أنواع السلاسل النصية
    static std::wstring stringToWString(const std::string& str);
    static std::string wStringToString(const std::wstring& wstr);

    // تنفيذ أوامر النظام
    static bool executeCommand(const std::string& command, std::string& output);
    static bool executeCommandSilently(const std::string& command);

    // وظائف مساعدة للملفات
    static bool fileExists(const std::string& filePath);
    static bool directoryExists(const std::string& dirPath);
    static bool createDirectory(const std::string& dirPath);
    static std::vector<std::string> getFilesInDirectory(const std::string& dirPath, const std::string& extension = "");

    // دوال خاصة بمسارات النظام
    static std::string getTempDirectory();
    static std::string getAppDataDirectory();
    static std::string getProgramFilesDirectory();
};

#endif // OSHELPER_H