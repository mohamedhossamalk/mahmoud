#include "../include/utils/oshelper.h"
#include "../include/utils/logger.h"
#include <Shlobj.h>
#include <memory>
#include <sstream>
#include <codecvt>
#include <locale>

std::string OSHelper::getOSVersion() {
    OSVERSIONINFOEX osInfo;
    ZeroMemory(&osInfo, sizeof(OSVERSIONINFOEX));
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    // ملاحظة: هذا قد لا يعمل بشكل جيد في Windows 8.1 وما فوق
    // في إنتاج حقيقي، قد ترغب في استخدام نهج مختلف مثل RtlGetVersion
#pragma warning(disable: 4996)
    GetVersionEx((OSVERSIONINFO*)&osInfo);

    std::stringstream ss;
    ss << "Windows ";

    if (osInfo.dwMajorVersion == 10) {
        if (osInfo.dwBuildNumber >= 22000) {
            ss << "11";
        }
        else {
            ss << "10";
        }
    }
    else if (osInfo.dwMajorVersion == 6) {
        if (osInfo.dwMinorVersion == 3)
            ss << "8.1";
        else if (osInfo.dwMinorVersion == 2)
            ss << "8";
        else if (osInfo.dwMinorVersion == 1)
            ss << "7";
        else
            ss << "Vista";
    }
    else if (osInfo.dwMajorVersion == 5) {
        if (osInfo.dwMinorVersion == 2)
            ss << "Server 2003";
        else if (osInfo.dwMinorVersion == 1)
            ss << "XP";
        else
            ss << "2000";
    }

    if (osInfo.wProductType == VER_NT_WORKSTATION)
        ss << " Professional";
    else
        ss << " Server";

    ss << " (Build " << osInfo.dwBuildNumber << ")";

    return ss.str();
}

std::string OSHelper::getComputerName() {
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer) / sizeof(wchar_t);

    if (GetComputerNameW(buffer, &size)) {
        return wStringToString(buffer);
    }

    return "Unknown";
}

std::string OSHelper::getUserName() {
    wchar_t buffer[257]; // UNLEN + 1
    DWORD size = sizeof(buffer) / sizeof(wchar_t);

    if (GetUserNameW(buffer, &size)) {
        return wStringToString(buffer);
    }

    return "Unknown";
}

bool OSHelper::isAdministrator() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID administratorsGroup;

    isAdmin = AllocateAndInitializeSid(
        &ntAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &administratorsGroup);

    if (isAdmin) {
        if (!CheckTokenMembership(NULL, administratorsGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(administratorsGroup);
    }

    return isAdmin != FALSE;
}

std::wstring OSHelper::stringToWString(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string OSHelper::wStringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

bool OSHelper::executeCommand(const std::string& command, std::string& output) {
    output.clear();

    // إنشاء أنابيب لالتقاط الإخراج
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE readPipe, writePipe;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
        Logger::log("Failed to create pipe for command execution");
        return false;
    }

    // إعداد معلومات بدء العملية
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = NULL;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;

    // تحويل الأمر إلى نسخة غير ثابتة
    char* cmdline = _strdup(command.c_str());

    // تنفيذ الأمر
    bool success = CreateProcessA(
        NULL,       // لا نستخدم اسم ملف تنفيذي محدد
        cmdline,    // سطر الأوامر
        NULL,       // لا يتم استخدام أمان العملية
        NULL,       // لا يتم استخدام أمان الموضوع
        TRUE,       // تمكين وراثة المقابض
        CREATE_NO_WINDOW, // إنشاء العملية بدون نافذة
        NULL,       // استخدام البيئة الوالدة
        NULL,       // استخدام دليل الوالد الحالي
        &si,        // معلومات بدء التشغيل
        &pi         // معلومات العملية
    );

    free(cmdline);

    if (!success) {
        Logger::log("Failed to execute command: " + command);
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        return false;
    }

    // إغلاق مقبض الكتابة لأننا لا نحتاجه
    CloseHandle(writePipe);

    // قراءة الإخراج
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    DWORD bytesRead;

    while (ReadFile(readPipe, buffer, BUFFER_SIZE - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        output += buffer;
    }

    // إغلاق المقابض المفتوحة
    CloseHandle(readPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

bool OSHelper::executeCommandSilently(const std::string& command) {
    std::string output;
    return executeCommand(command, output);
}

bool OSHelper::fileExists(const std::string& filePath) {
    return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

bool OSHelper::directoryExists(const std::string& dirPath) {
    return std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath);
}

bool OSHelper::createDirectory(const std::string& dirPath) {
    try {
        return std::filesystem::create_directories(dirPath);
    }
    catch (const std::filesystem::filesystem_error& e) {
        Logger::log("Failed to create directory: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> OSHelper::getFilesInDirectory(const std::string& dirPath, const std::string& extension) {
    std::vector<std::string> files;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (std::filesystem::is_regular_file(entry)) {
                // إذا تم تحديد امتداد، تحقق من امتداد الملف
                if (!extension.empty()) {
                    if (entry.path().extension() == extension) {
                        files.push_back(entry.path().string());
                    }
                }
                else {
                    files.push_back(entry.path().string());
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        Logger::log("Error reading directory: " + std::string(e.what()));
    }

    return files;
}

std::string OSHelper::getTempDirectory() {
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath)) {
        return wStringToString(std::wstring(tempPath));
    }
    return "";
}

std::string OSHelper::getAppDataDirectory() {
    wchar_t* appDataPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataPath))) {
        std::string result = wStringToString(appDataPath);
        CoTaskMemFree(appDataPath);
        return result;
    }
    return "";
}

std::string OSHelper::getProgramFilesDirectory() {
    wchar_t* programFilesPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &programFilesPath))) {
        std::string result = wStringToString(programFilesPath);
        CoTaskMemFree(programFilesPath);
        return result;
    }
    return "";
}