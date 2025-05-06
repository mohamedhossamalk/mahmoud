#ifndef PROCESSTAB_H
#define PROCESSTAB_H

#include <Windows.h>
#include <CommCtrl.h>
#include <vector>
#include "../system/processmanager.h"

#define IDC_PROCESS_LIST         4001
#define IDC_REFRESH_BUTTON       4002
#define IDC_SET_PRIORITY_BUTTON  4003
#define IDC_TERMINATE_BUTTON     4004
#define IDC_PRIORITY_COMBO       4005
#define IDC_PROCESS_INFO_STATIC  4006

class ProcessTab {
public:
    ProcessTab(HWND hParent, HINSTANCE hInstance, ProcessManager* processManager);
    ~ProcessTab();

    // إنشاء علامة التبويب
    bool Create(int x, int y, int width, int height);

    // إظهار/إخفاء/تحديث علامة التبويب
    void Show();
    void Hide();
    void Update();

    // إعادة تحجيم علامة التبويب
    void Resize(int width, int height);

private:
    // وظيفة إجراءات علامة التبويب
    static LRESULT CALLBACK TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // وظائف مساعدة
    void FillProcessList();
    void UpdateProcessInfo();

    // مقابض النافذة والعناصر
    HWND hParent;
    HINSTANCE hInstance;
    HWND hWnd;
    HWND hProcessList;
    HWND hRefreshButton;
    HWND hSetPriorityButton;
    HWND hTerminateButton;
    HWND hPriorityCombo;
    HWND hProcessInfoStatic;

    // مدير العمليات
    ProcessManager* processManager;

    // قائمة العمليات الحالية
    std::vector<ProcessInfo> processes;

    // إجراء النافذة السابق
    WNDPROC oldTabProc;
};

#endif // PROCESSTAB_H