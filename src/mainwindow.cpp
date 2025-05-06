#include "../include/mainwindow.h"
#include "../include/utils/logger.h"
#include <sstream>

// تسجيل فئة النافذة
const wchar_t* const CLASS_NAME = L"SystemMonitorWindowClass";

MainWindow::MainWindow(HINSTANCE hInstance) : hInstance(hInstance), hWnd(NULL) {
    // تسجيل فئة النافذة
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // إنشاء مراقب النظام
    systemMonitor = std::make_unique<SystemMonitor>();
}

MainWindow::~MainWindow() {
    // إلغاء تسجيل فئة النافذة عند الخروج
    UnregisterClass(CLASS_NAME, hInstance);
}

bool MainWindow::Create(const wchar_t* windowName, DWORD style, DWORD exStyle,
    int x, int y, int width, int height,
    HWND parent, HMENU menu) {
    // إنشاء النافذة
    hWnd = CreateWindowEx(
        exStyle,
        CLASS_NAME,
        windowName,
        style,
        x, y, width, height,
        parent, menu, hInstance, this
    );

    if (!hWnd) {
        Logger::log("Failed to create main window");
        return false;
    }

    // إنشاء عناصر التحكم
    CreateControls();

    // إنشاء مؤقت لتحديث معلومات النظام (كل ثانية)
    SetTimer(hWnd, IDC_TIMER, 1000, NULL);

    return true;
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = NULL;

    if (uMsg == WM_NCCREATE) {
        // استخراج إشارة إلى كائن MainWindow من بيانات الإنشاء
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (MainWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else {
        // استرداد إشارة إلى كائن MainWindow من بيانات النافذة
        pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        // توجيه الرسالة إلى وظيفة معالجة الرسائل في الكائن
        return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
    }
    else {
        // إذا لم نتمكن من استرداد إشارة إلى الكائن، استخدم معالجة افتراضية
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT MainWindow::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        return 0;

    case WM_SIZE: {
        // إعادة تحجيم عناصر التحكم عندما يتم تغيير حجم النافذة
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        // إعادة تحجيم شريط التبويب
        SetWindowPos(hTabControl, NULL, 0, 0, width, height - 20, SWP_NOMOVE | SWP_NOZORDER);

        // إعادة تحجيم شريط الحالة
        SendMessage(hStatusBar, WM_SIZE, 0, 0);

        // إعادة تحجيم علامات التبويب
        if (ramTab) ramTab->Resize(width, height - 40);
        if (cacheTab) cacheTab->Resize(width, height - 40);
        if (processTab) processTab->Resize(width, height - 40);

        return 0;
    }

    case WM_NOTIFY: {
        NMHDR* nmhdr = (NMHDR*)lParam;

        if (nmhdr->hwndFrom == hTabControl && nmhdr->code == TCN_SELCHANGE) {
            // تم تغيير علامة التبويب المحددة
            int selectedTab = TabCtrl_GetCurSel(hTabControl);

            // إخفاء جميع علامات التبويب
            if (ramTab) ramTab->Hide();
            if (cacheTab) cacheTab->Hide();
            if (processTab) processTab->Hide();

            // إظهار علامة التبويب المحددة
            switch (selectedTab) {
            case 0: // علامة تبويب الذاكرة
                if (ramTab) ramTab->Show();
                break;
            case 1: // علامة تبويب الكاش
                if (cacheTab) cacheTab->Show();
                break;
            case 2: // علامة تبويب العمليات
                if (processTab) processTab->Show();
                break;
            }
        }

        return 0;
    }

    case WM_TIMER:
        if (wParam == IDC_TIMER) {
            // تحديث معلومات النظام كل ثانية
            UpdateSystemInfo();
        }
        return 0;

    case WM_DESTROY:
        // إلغاء المؤقت وإنهاء التطبيق
        KillTimer(hwnd, IDC_TIMER);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MainWindow::CreateControls() {
    // إنشاء شريط التبويب
    hTabControl = CreateWindow(
        WC_TABCONTROL, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 0, 0,
        hWnd, (HMENU)IDC_TAB_CONTROL, hInstance, NULL
    );

    // إضافة علامات التبويب
    TCITEM tie;
    tie.mask = TCIF_TEXT;

    tie.pszText = const_cast<LPWSTR>(L"الذاكرة العشوائية");
    TabCtrl_InsertItem(hTabControl, 0, &tie);

    tie.pszText = const_cast<LPWSTR>(L"الكاش");
    TabCtrl_InsertItem(hTabControl, 1, &tie);

    tie.pszText = const_cast<LPWSTR>(L"العمليات");
    TabCtrl_InsertItem(hTabControl, 2, &tie);

    // إنشاء شريط الحالة
    hStatusBar = CreateWindow(
        STATUSCLASSNAME, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hWnd, (HMENU)IDC_STATUS_BAR, hInstance, NULL
    );

    // تحديد قسم شريط الحالة
    int statwidths[] = { -1 };
    SendMessage(hStatusBar, SB_SETPARTS, 1, (LPARAM)statwidths);
    SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)L"جاهز");

    // الحصول على أبعاد عنصر التحكم
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    // إنشاء علامات التبويب
    ramTab = std::make_unique<RamTab>(hWnd, hInstance, &systemMonitor->getRamMonitor());
    ramTab->Create(0, 24, width, height - 44);

    cacheTab = std::make_unique<CacheTab>(hWnd, hInstance, &systemMonitor->getCacheManager());
    cacheTab->Create(0, 24, width, height - 44);
    cacheTab->Hide();

    processTab = std::make_unique<ProcessTab>(hWnd, hInstance, &systemMonitor->getProcessManager());
    processTab->Create(0, 24, width, height - 44);
    processTab->Hide();

    // تحديد علامة التبويب الأولى
    TabCtrl_SetCurSel(hTabControl, 0);
}

void MainWindow::UpdateSystemInfo() {
    // الحصول على معلومات النظام المحدثة
    SystemResources resources = systemMonitor->getSystemResources();

    // تحديث شريط الحالة بمعلومات النظام
    std::wstringstream statusText;
    statusText << L"المعالج: " << static_cast<int>(resources.cpuUsage) << L"% | "
        << L"الذاكرة: " << static_cast<int>((resources.usedRAM / resources.totalRAM) * 100) << L"% ("
        << static_cast<int>(resources.usedRAM) << L" GB / "
        << static_cast<int>(resources.totalRAM) << L" GB)";

    SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)statusText.str().c_str());

    // تحديث محتوى علامات التبويب
    if (ramTab) ramTab->Update();
    if (cacheTab) cacheTab->Update();
    if (processTab) processTab->Update();

    // تحديث سجل استخدام الذاكرة للرسوم البيانية
    systemMonitor->getRamMonitor().updateRAMUsageHistory();
}