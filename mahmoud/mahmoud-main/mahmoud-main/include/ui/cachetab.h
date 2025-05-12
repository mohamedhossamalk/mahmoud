#ifndef CACHETAB_H
#define CACHETAB_H

#include <Windows.h>
#include <CommCtrl.h>
#include "../memory/cachemanager.h"

#define IDC_CLEAR_DISK_CACHE     3001
#define IDC_CLEAR_DNS_CACHE      3002
#define IDC_CLEAR_BROWSER_CACHE  3003
#define IDC_CACHE_INFO_STATIC    3004
#define IDC_BROWSER_COMBO        3005

class CacheTab {
public:
    CacheTab(HWND hParent, HINSTANCE hInstance, CacheManager* cacheManager);
    ~CacheTab();

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

    // مقابض النافذة والعناصر
    HWND hParent;
    HINSTANCE hInstance;
    HWND hWnd;
    HWND hCacheInfoStatic;
    HWND hClearDiskCacheButton;
    HWND hClearDNSCacheButton;
    HWND hClearBrowserCacheButton;
    HWND hBrowserCombo;

    // مدير الكاش
    CacheManager* cacheManager;

    // إجراء النافذة السابق
    WNDPROC oldTabProc;
};

#endif // CACHETAB_H