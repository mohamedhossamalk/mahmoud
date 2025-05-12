#ifndef RAMTAB_H
#define RAMTAB_H

#include <Windows.h>
#include <CommCtrl.h>
#include "../memory/rammonitor.h"

#define IDC_RAM_PROGRESS        2001
#define IDC_RAM_INFO_STATIC     2002
#define IDC_OPTIMIZE_BUTTON     2003
#define IDC_RAM_GRAPH           2004

class RamTab {
public:
    RamTab(HWND hParent, HINSTANCE hInstance, RamMonitor* ramMonitor);
    ~RamTab();

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

    // وظائف الرسم
    void DrawRAMGraph(HDC hdc, RECT rect);

    // مقابض النافذة والعناصر
    HWND hParent;
    HINSTANCE hInstance;
    HWND hWnd;
    HWND hRAMProgressBar;
    HWND hRAMInfoStatic;
    HWND hOptimizeButton;
    HWND hRAMGraph;

    // مراقب الذاكرة
    RamMonitor* ramMonitor;

    // إجراء النافذة السابق
    WNDPROC oldTabProc;
};

#endif // RAMTAB_H