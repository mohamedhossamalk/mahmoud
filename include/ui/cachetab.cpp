#include "../include/ui/cachetab.h"
#include "../include/utils/logger.h"
#include <sstream>

CacheTab::CacheTab(HWND hParent, HINSTANCE hInstance, CacheManager* cacheManager)
    : hParent(hParent), hInstance(hInstance), cacheManager(cacheManager),
    hWnd(NULL), hCacheInfoStatic(NULL), hClearDiskCacheButton(NULL),
    hClearDNSCacheButton(NULL), hClearBrowserCacheButton(NULL), hBrowserCombo(NULL),
    oldTabProc(NULL) {
}

CacheTab::~CacheTab() {
    // يتم تدمير عناصر التحكم تلقائيًا عند تدمير النافذة الأم
}

bool CacheTab::Create(int x, int y, int width, int height) {
    // إنشاء النافذة الأساسية لعلامة التبويب
    hWnd = CreateWindow(
        L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, width, height,
        hParent, NULL, hInstance, NULL
    );

    if (!hWnd) {
        Logger::log("Failed to create Cache tab window");
        return false;
    }

    // تعيين إجراء النافذة المخصص
    oldTabProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)TabProc);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

    // إنشاء الملصق النصي لمعلومات الكاش
    hCacheInfoStatic = CreateWindow(
        L"STATIC", L"جاري تحميل معلومات الكاش...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 30, width - 40, 70,
        hWnd, (HMENU)IDC_CACHE_INFO_STATIC, hInstance, NULL
    );

    // إنشاء أزرار تنظيف الكاش
    hClearDiskCacheButton = CreateWindow(
        L"BUTTON", L"تنظيف كاش القرص",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 120, 200, 30,
        hWnd, (HMENU)IDC_CLEAR_DISK_CACHE, hInstance, NULL
    );

    hClearDNSCacheButton = CreateWindow(
        L"BUTTON", L"تنظيف كاش DNS",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 160, 200, 30,
        hWnd, (HMENU)IDC_CLEAR_DNS_CACHE, hInstance, NULL
    );

    // إنشاء قائمة منسدلة للمتصفحات
    hBrowserCombo = CreateWindow(
        L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        20, 200, 200, 100,
        hWnd, (HMENU)IDC_BROWSER_COMBO, hInstance, NULL
    );

    // إضافة المتصفحات إلى القائمة المنسدلة
    SendMessage(hBrowserCombo, CB_ADDSTRING, 0, (LPARAM)L"جميع المتصفحات");
    SendMessage(hBrowserCombo, CB_ADDSTRING, 0, (LPARAM)L"Google Chrome");
    SendMessage(hBrowserCombo, CB_ADDSTRING, 0, (LPARAM)L"Mozilla Firefox");
    SendMessage(hBrowserCombo, CB_ADDSTRING, 0, (LPARAM)L"Microsoft Edge");
    SendMessage(hBrowserCombo, CB_SETCURSEL, 0, 0);

    hClearBrowserCacheButton = CreateWindow(
        L"BUTTON", L"تنظيف كاش المتصفح",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        240, 200, 200, 30,
        hWnd, (HMENU)IDC_CLEAR_BROWSER_CACHE, hInstance, NULL
    );

    // تحديث المعلومات
    Update();

    return true;
}

void CacheTab::Show() {
    ShowWindow(hWnd, SW_SHOW);
}

void CacheTab::Hide() {
    ShowWindow(hWnd, SW_HIDE);
}

void CacheTab::Update() {
    if (!hWnd || !cacheManager) return;

    // حساب حجم كاش القرص
    size_t diskCacheSize = cacheManager->getDiskCacheSize();
    double cacheSizeInMB = diskCacheSize / (1024.0 * 1024.0);

    // تحديث ملصق المعلومات
    std::wstringstream infoText;
    infoText << L"معلومات الكاش:\r\n\r\n"
        << L"حجم كاش المتصفحات التقريبي: " << cacheSizeInMB << L" ميجابايت\r\n\r\n"
        << L"يمكنك تنظيف الكاش المختلفة باستخدام الأزرار أدناه.";

    SetWindowText(hCacheInfoStatic, infoText.str().c_str());
}

void CacheTab::Resize(int width, int height) {
    if (!hWnd) return;

    // إعادة تحجيم النافذة الرئيسية لعلامة التبويب
    SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

    // إعادة تحجيم عناصر التحكم
    SetWindowPos(hCacheInfoStatic, NULL, 20, 30, width - 40, 70, SWP_NOZORDER);
    // الأزرار والقوائم المنسدلة لا تحتاج إلى تغيير الحجم
}

LRESULT CALLBACK CacheTab::TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CacheTab* pThis = (CacheTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (pThis) {
        return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
    }
    else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT CacheTab::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD(wParam) == IDC_CLEAR_DISK_CACHE) {
                // تنظيف كاش القرص
                if (cacheManager) {
                    if (cacheManager->flushDiskCache()) {
                        MessageBox(hwnd, L"تم تنظيف كاش القرص بنجاح!", L"تنظيف الكاش", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(hwnd, L"فشل في تنظيف كاش القرص.", L"خطأ", MB_OK | MB_ICONERROR);
                    }
                    Update();
                }
                return 0;
            }
            else if (LOWORD(wParam) == IDC_CLEAR_DNS_CACHE) {
                // تنظيف كاش DNS
                if (cacheManager) {
                    if (cacheManager->flushDNSCache()) {
                        MessageBox(hwnd, L"تم تنظيف كاش DNS بنجاح!", L"تنظيف الكاش", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(hwnd, L"فشل في تنظيف كاش DNS.", L"خطأ", MB_OK | MB_ICONERROR);
                    }
                }
                return 0;
            }
            else if (LOWORD(wParam) == IDC_CLEAR_BROWSER_CACHE) {
                // تنظيف كاش المتصفح
                if (cacheManager) {
                    // الحصول على المتصفح المحدد
                    int selectedIndex = (int)SendMessage(hBrowserCombo, CB_GETCURSEL, 0, 0);
                    std::string browserName;

                    if (selectedIndex > 0) {
                        wchar_t browserText[100];
                        SendMessage(hBrowserCombo, CB_GETLBTEXT, selectedIndex, (LPARAM)browserText);

                        // تحويل اسم المتصفح إلى ASCII
                        char asciiText[100];
                        WideCharToMultiByte(CP_ACP, 0, browserText, -1, asciiText, sizeof(asciiText), NULL, NULL);
                        browserName = asciiText;

                        // تحويل اسم المتصفح إلى الاسم المستخدم في فئة CacheManager
                        if (browserName == "Google Chrome") browserName = "chrome";
                        else if (browserName == "Mozilla Firefox") browserName = "firefox";
                        else if (browserName == "Microsoft Edge") browserName = "edge";
                    }

                    if (cacheManager->clearBrowserCache(browserName)) {
                        MessageBox(hwnd, L"تم تنظيف كاش المتصفح بنجاح!", L"تنظيف الكاش", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(hwnd, L"فشل في تنظيف كاش المتصفح.", L"خطأ", MB_OK | MB_ICONERROR);
                    }
                    Update();
                }
                return 0;
            }
        }
        break;
    }

    return CallWindowProc(oldTabProc, hwnd, uMsg, wParam, lParam);
}