#include "../include/mainwindow.h"
#include "../include/utils/logger.h"
#include "../include/utils/oshelper.h"
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // تهيئة نظام السجل
    Logger::initialize("SystemMonitor.log");
    Logger::log("Application started");

    // تسجيل معلومات النظام
    Logger::log("OS: " + OSHelper::getOSVersion());
    Logger::log("Computer Name: " + OSHelper::getComputerName());
    Logger::log("User Name: " + OSHelper::getUserName());
    Logger::log("Running as Administrator: " + std::string(OSHelper::isAdministrator() ? "Yes" : "No"));
    // إنشاء وإظهار النافذة
    if (!mainWindow.Create(L"مدير موارد النظام", WS_OVERLAPPEDWINDOW, 0,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL)) {
        Logger::log("Failed to create main window");
        return 0;
    }

    Logger::log("Main window created successfully");

    ShowWindow(mainWindow.GetHandle(), nCmdShow);
    UpdateWindow(mainWindow.GetHandle());

    // دورة معالجة الرسائل
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Logger::log("Application terminated");
    return (int)msg.wParam;
}