#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Windows.h>
#include <CommCtrl.h>
#include <memory>
#include <vector>
#include <string>
#include "system/systemmonitor.h"
#include "ui/ramtab.h"
#include "ui/cachetab.h"
#include "ui/processtab.h"

#define IDC_TAB_CONTROL    1001
#define IDC_STATUS_BAR     1002
#define IDC_TIMER          1003

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance);
    ~MainWindow();

    // إنشاء النافذة
    bool Create(const wchar_t* windowName, DWORD style, DWORD exStyle,
        int x, int y, int width, int height,
        HWND parent, HMENU menu);

    // الحصول على مقبض النافذة
    HWND GetHandle() const { return hWnd; }

private:
    // وظيفة إجراءات النافذة
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // وظائف الإنشاء والتحديث
    void CreateControls();
    void UpdateSystemInfo();

    // مقابض النافذة والعناصر
    HINSTANCE hInstance;
    HWND hWnd;
    HWND hTabControl;
    HWND hStatusBar;

    // علامات التبويب
    std::unique_ptr<RamTab> ramTab;
    std::unique_ptr<CacheTab> cacheTab;
    std::unique_ptr<ProcessTab> processTab;

    // مراقب النظام
    std::unique_ptr<SystemMonitor> systemMonitor;
};

#endif // MAINWINDOW_H