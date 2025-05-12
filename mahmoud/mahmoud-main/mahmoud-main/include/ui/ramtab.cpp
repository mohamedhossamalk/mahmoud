#include "../include/ui/ramtab.h"
#include "../include/utils/logger.h"
#include <sstream>

RamTab::RamTab(HWND hParent, HINSTANCE hInstance, RamMonitor* ramMonitor)
    : hParent(hParent), hInstance(hInstance), ramMonitor(ramMonitor),
    hWnd(NULL), hRAMProgressBar(NULL), hRAMInfoStatic(NULL), hOptimizeButton(NULL), hRAMGraph(NULL),
    oldTabProc(NULL) {
}

RamTab::~RamTab() {
    // يتم تدمير عناصر التحكم تلقائيًا عند تدمير النافذة الأم
}

bool RamTab::Create(int x, int y, int width, int height) {
    // إنشاء النافذة الأساسية لعلامة التبويب
    hWnd = CreateWindow(
        L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, width, height,
        hParent, NULL, hInstance, NULL
    );

    if (!hWnd) {
        Logger::log("Failed to create RAM tab window");
        return false;
    }

    // تعيين إجراء النافذة المخصص
    oldTabProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)TabProc);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

    // إنشاء شريط تقدم الذاكرة
    hRAMProgressBar = CreateWindow(
        PROGRESS_CLASS, L"",
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 30, width - 40, 30,
        hWnd, (HMENU)IDC_RAM_PROGRESS, hInstance, NULL
    );
    SendMessage(hRAMProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    // إنشاء الملصق النصي لمعلومات الذاكرة
    hRAMInfoStatic = CreateWindow(
        L"STATIC", L"جاري تحميل معلومات الذاكرة...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 70, width - 40, 100,
        hWnd, (HMENU)IDC_RAM_INFO_STATIC, hInstance, NULL
    );

    // إنشاء زر تحسين الذاكرة
    hOptimizeButton = CreateWindow(
        L"BUTTON", L"تحسين استخدام الذاكرة",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 180, 200, 30,
        hWnd, (HMENU)IDC_OPTIMIZE_BUTTON, hInstance, NULL
    );

    // إنشاء منطقة رسم الرسم البياني
    hRAMGraph = CreateWindow(
        L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        20, 230, width - 40, 150,
        hWnd, (HMENU)IDC_RAM_GRAPH, hInstance, NULL
    );

    // تحديث المعلومات
    Update();

    return true;
}

void RamTab::Show() {
    ShowWindow(hWnd, SW_SHOW);
}

void RamTab::Hide() {
    ShowWindow(hWnd, SW_HIDE);
}

void RamTab::Update() {
    if (!hWnd || !ramMonitor) return;

    // الحصول على معلومات الذاكرة
    RAMInfo ramInfo = ramMonitor->getRAMInfo();

    // تحديث شريط التقدم
    SendMessage(hRAMProgressBar, PBM_SETPOS, (int)ramInfo.memoryLoad, 0);

    // تحديث ملصق المعلومات
    std::wstringstream infoText;
    infoText << L"إجمالي الذاكرة الفيزيائية: " << ramInfo.totalPhysical << L" جيجابايت\r\n"
        << L"الذاكرة المستخدمة: " << ramInfo.usedPhysical << L" جيجابايت (" << ramInfo.memoryLoad << L"%)\r\n"
        << L"الذاكرة المتاحة: " << ramInfo.availablePhysical << L" جيجابايت\r\n\r\n"
        << L"إجمالي الذاكرة الافتراضية: " << ramInfo.totalVirtual << L" جيجابايت\r\n"
        << L"الذاكرة الافتراضية المستخدمة: " << ramInfo.usedVirtual << L" جيجابايت\r\n"
        << L"الذاكرة الافتراضية المتاحة: " << ramInfo.availableVirtual << L" جيجابايت";

    SetWindowText(hRAMInfoStatic, infoText.str().c_str());

    // تحديث الرسم البياني
    InvalidateRect(hRAMGraph, NULL, TRUE);
}

void RamTab::Resize(int width, int height) {
    if (!hWnd) return;

    // إعادة تحجيم النافذة الرئيسية لعلامة التبويب
    SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

    // إعادة تحجيم عناصر التحكم
    SetWindowPos(hRAMProgressBar, NULL, 20, 30, width - 40, 30, SWP_NOZORDER);
    SetWindowPos(hRAMInfoStatic, NULL, 20, 70, width - 40, 100, SWP_NOZORDER);
    // الزر لا يحتاج إلى تغيير حجمه
    SetWindowPos(hRAMGraph, NULL, 20, 230, width - 40, 150, SWP_NOZORDER);
}

LRESULT CALLBACK RamTab::TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    RamTab* pThis = (RamTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (pThis) {
        return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
    }
    else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT RamTab::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_OPTIMIZE_BUTTON && HIWORD(wParam) == BN_CLICKED) {
            // زر تحسين الذاكرة تم النقر عليه
            if (ramMonitor) {
                if (ramMonitor->optimizeRAM()) {
                    MessageBox(hwnd, L"تم تحسين الذاكرة بنجاح!", L"تحسين الذاكرة", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    MessageBox(hwnd, L"فشل في تحسين الذاكرة.", L"خطأ", MB_OK | MB_ICONERROR);
                }
                Update();
            }
            return 0;
        }
        break;

    case WM_DRAWITEM:
        if (wParam == IDC_RAM_GRAPH) {
            // رسم الرسم البياني للذاكرة
            DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
            DrawRAMGraph(dis->hDC, dis->rcItem);
            return TRUE;
        }
        break;
    }

    return CallWindowProc(oldTabProc, hwnd, uMsg, wParam, lParam);
}

void RamTab::DrawRAMGraph(HDC hdc, RECT rect) {
    if (!ramMonitor) return;

    // الحصول على بيانات استخدام الذاكرة
    std::vector<RAMUsageRecord> history = ramMonitor->getRAMUsageHistory();
    if (history.empty()) return;

    // تهيئة منطقة الرسم
    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // إنشاء القلم للرسم
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    // إنشاء القلم للشبكة
    HPEN hGridPen = CreatePen(PS_DOT, 1, RGB(200, 200, 200));

    // حساب النطاق الزمني
    ULONGLONG latestTime = history.back().timestamp;
    ULONGLONG earliestTime = history.front().timestamp;
    ULONGLONG timeRange = latestTime - earliestTime;

    if (timeRange == 0) timeRange = 1; // تجنب القسمة على صفر

    // رسم الشبكة
    SelectObject(hdc, hGridPen);

    // خطوط أفقية (نسب مئوية)
    for (int i = 0; i <= 100; i += 20) {
        int y = rect.bottom - (i * (rect.bottom - rect.top) / 100);
        MoveToEx(hdc, rect.left, y, NULL);
        LineTo(hdc, rect.right, y);

        // وضع تسميات النسب المئوية
        std::wstringstream ss;
        ss << i << L"%";
        TextOut(hdc, rect.left + 5, y - 10, ss.str().c_str(), (int)ss.str().length());
    }

    // العودة إلى قلم الرسم البياني
    SelectObject(hdc, hPen);

    // رسم منحنى استخدام الذاكرة
    bool firstPoint = true;
    int lastX = 0;
    int lastY = 0;

    for (const auto& record : history) {
        // حساب الإحداثيات
        int x = rect.left + (int)(((record.timestamp - earliestTime) * (rect.right - rect.left)) / timeRange);
        int y = rect.bottom - (int)((record.memoryUsage * (rect.bottom - rect.top)) / 100);

        if (firstPoint) {
            MoveToEx(hdc, x, y, NULL);
            firstPoint = false;
        }
        else {
            LineTo(hdc, x, y);
        }

        lastX = x;
        lastY = y;
    }

    // إضافة المعلومات الحالية
    if (!history.empty()) {
        std::wstringstream ss;
        ss << L"استخدام الذاكرة الحالي: " << (int)history.back().memoryUsage << L"%";
        TextOut(hdc, lastX - 150, lastY - 20, ss.str().c_str(), (int)ss.str().length());
    }

    // تنظيف الموارد
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    DeleteObject(hGridPen);
}