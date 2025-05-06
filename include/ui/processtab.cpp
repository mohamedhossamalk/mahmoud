#include "../include/ui/processtab.h"
#include "../include/utils/logger.h"
#include <sstream>
#include <iomanip>

ProcessTab::ProcessTab(HWND hParent, HINSTANCE hInstance, ProcessManager* processManager)
    : hParent(hParent), hInstance(hInstance), processManager(processManager),
    hWnd(NULL), hProcessList(NULL), hRefreshButton(NULL),
    hSetPriorityButton(NULL), hTerminateButton(NULL), hPriorityCombo(NULL),
    hProcessInfoStatic(NULL), oldTabProc(NULL) {
}

ProcessTab::~ProcessTab() {
    // يتم تدمير عناصر التحكم تلقائيًا عند تدمير النافذة الأم
}

bool ProcessTab::Create(int x, int y, int width, int height) {
    // إنشاء النافذة الأساسية لعلامة التبويب
    hWnd = CreateWindow(
        L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, width, height,
        hParent, NULL, hInstance, NULL
    );

    if (!hWnd) {
        Logger::log("Failed to create Process tab window");
        return false;
    }

    // تهيئة عناصر التحكم المشتركة
    InitCommonControls();

    // تعيين إجراء النافذة المخصص
    oldTabProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)TabProc);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

    // إنشاء قائمة العمليات
    hProcessList = CreateWindowEx(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        20, 30, width - 40, height / 2 - 40,
        hWnd, (HMENU)IDC_PROCESS_LIST, hInstance, NULL
    );

    // إعداد أعمدة قائمة العمليات
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;

    lvc.iSubItem = 0;
    lvc.cx = 200;
    lvc.pszText = const_cast<LPWSTR>(L"اسم العملية");
    ListView_InsertColumn(hProcessList, 0, &lvc);

    lvc.iSubItem = 1;
    lvc.cx = 80;
    lvc.pszText = const_cast<LPWSTR>(L"معرف العملية");
    ListView_InsertColumn(hProcessList, 1, &lvc);

    lvc.iSubItem = 2;
    lvc.cx = 120;
    lvc.pszText = const_cast<LPWSTR>(L"استخدام الذاكرة");
    ListView_InsertColumn(hProcessList, 2, &lvc);

    lvc.iSubItem = 3;
    lvc.cx = 100;
    lvc.pszText = const_cast<LPWSTR>(L"استخدام المعالج");
    ListView_InsertColumn(hProcessList, 3, &lvc);

    lvc.iSubItem = 4;
    lvc.cx = 80;
    lvc.pszText = const_cast<LPWSTR>(L"الحالة");
    ListView_InsertColumn(hProcessList, 4, &lvc);

    // إنشاء زر التحديث
    hRefreshButton = CreateWindow(
        L"BUTTON", L"تحديث القائمة",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, height / 2, 150, 30,
        hWnd, (HMENU)IDC_REFRESH_BUTTON, hInstance, NULL
    );

    // إنشاء قائمة منسدلة لاختيار الأولوية
    hPriorityCombo = CreateWindow(
        L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        200, height / 2, 150, 150,
        hWnd, (HMENU)IDC_PRIORITY_COMBO, hInstance, NULL
    );

    // إضافة خيارات الأولوية
    SendMessage(hPriorityCombo, CB_ADDSTRING, 0, (LPARAM)L"خامل");
    SendMessage(hPriorityCombo, CB_ADDSTRING, 0, (LPARAM)L"أقل من عادي");
    SendMessage(hPriorityCombo, CB_ADDSTRING, 0, (LPARAM)L"عادي");
    SendMessage(hPriorityCombo, CB_ADDSTRING, 0, (LPARAM)L"أعلى من عادي");
    SendMessage(hPriorityCombo, CB_ADDSTRING, 0, (LPARAM)L"مرتفع");
    SendMessage(hPriorityCombo, CB_ADDSTRING, 0, (LPARAM)L"وقت حقيقي");
    SendMessage(hPriorityCombo, CB_SETCURSEL, 2, 0); // اختيار "عادي" كقيمة افتراضية

    // إنشاء زر تعيين الأولوية
    hSetPriorityButton = CreateWindow(
        L"BUTTON", L"تعيين الأولوية",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        360, height / 2, 150, 30,
        hWnd, (HMENU)IDC_SET_PRIORITY_BUTTON, hInstance, NULL
    );

    // إنشاء زر إنهاء العملية
    hTerminateButton = CreateWindow(
        L"BUTTON", L"إنهاء العملية",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        520, height / 2, 150, 30,
        hWnd, (HMENU)IDC_TERMINATE_BUTTON, hInstance, NULL
    );

    // إنشاء الملصق النصي لمعلومات العملية
    hProcessInfoStatic = CreateWindow(
        L"STATIC", L"اختر عملية من القائمة لعرض معلوماتها",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, height / 2 + 50, width - 40, height / 2 - 60,
        hWnd, (HMENU)IDC_PROCESS_INFO_STATIC, hInstance, NULL
    );

    // تحديث قائمة العمليات
    FillProcessList();

    return true;
}

void ProcessTab::Show() {
    ShowWindow(hWnd, SW_SHOW);
}

void ProcessTab::Hide() {
    ShowWindow(hWnd, SW_HIDE);
}

void ProcessTab::Update() {
    if (!hWnd || !processManager) return;

    // تحديث قائمة العمليات كل بضع ثوانٍ لتجنب استهلاك الموارد
    static int updateCounter = 0;
    if (updateCounter++ % 3 == 0) { // تحديث كل 3 ثوانٍ
        FillProcessList();
    }
}

void ProcessTab::Resize(int width, int height) {
    if (!hWnd) return;

    // إعادة تحجيم النافذة الرئيسية لعلامة التبويب
    SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

    // إعادة تحجيم قائمة العمليات
    SetWindowPos(hProcessList, NULL, 20, 30, width - 40, height / 2 - 40, SWP_NOZORDER);

    // إعادة تحجيم وإعادة تموضع أزرار التحكم
    SetWindowPos(hRefreshButton, NULL, 20, height / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(hPriorityCombo, NULL, 200, height / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(hSetPriorityButton, NULL, 360, height / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(hTerminateButton, NULL, 520, height / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    // إعادة تحجيم ملصق المعلومات
    SetWindowPos(hProcessInfoStatic, NULL, 20, height / 2 + 50, width - 40, height / 2 - 60, SWP_NOZORDER);
}

LRESULT CALLBACK ProcessTab::TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ProcessTab* pThis = (ProcessTab*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (pThis) {
        return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
    }
    else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT ProcessTab::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD(wParam) == IDC_REFRESH_BUTTON) {
                // تحديث قائمة العمليات
                FillProcessList();
                return 0;
            }
            else if (LOWORD(wParam) == IDC_SET_PRIORITY_BUTTON) {
                // تعيين أولوية العملية المحددة
                int selectedIndex = ListView_GetNextItem(hProcessList, -1, LVNI_SELECTED);
                if (selectedIndex >= 0 && selectedIndex < processes.size()) {
                    // الحصول على معرف العملية من القائمة
                    DWORD processId = processes[selectedIndex].id;

                    // الحصول على الأولوية المحددة
                    int priorityIndex = (int)SendMessage(hPriorityCombo, CB_GETCURSEL, 0, 0);
                    PriorityClass priorityClass;

                    switch (priorityIndex) {
                    case 0: priorityClass = PriorityClass::Idle; break;
                    case 1: priorityClass = PriorityClass::BelowNormal; break;
                    case 2: priorityClass = PriorityClass::Normal; break;
                    case 3: priorityClass = PriorityClass::AboveNormal; break;
                    case 4: priorityClass = PriorityClass::High; break;
                    case 5: priorityClass = PriorityClass::RealTime; break;
                    default: priorityClass = PriorityClass::Normal;
                    }

                    // تعيين أولوية العملية
                    if (processManager->setProcessPriority(processId, priorityClass)) {
                        MessageBox(hwnd, L"تم تعيين أولوية العملية بنجاح!", L"تعيين الأولوية", MB_OK | MB_ICONINFORMATION);
                        FillProcessList();
                    }
                    else {
                        MessageBox(hwnd, L"فشل في تعيين أولوية العملية.", L"خطأ", MB_OK | MB_ICONERROR);
                    }
                }
                else {
                    MessageBox(hwnd, L"الرجاء اختيار عملية من القائمة أولاً.", L"خطأ", MB_OK | MB_ICONWARNING);
                }
                return 0;
            }
            else if (LOWORD(wParam) == IDC_TERMINATE_BUTTON) {
                // إنهاء العملية المحددة
                int selectedIndex = ListView_GetNextItem(hProcessList, -1, LVNI_SELECTED);
                if (selectedIndex >= 0 && selectedIndex < processes.size()) {
                    // الحصول على معرف العملية من القائمة
                    DWORD processId = processes[selectedIndex].id;
                    std::wstring processName;

                    // تحويل اسم العملية إلى Unicode
                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                    processName = converter.from_bytes(processes[selectedIndex].name);

                    // تأكيد إنهاء العملية
                    std::wstringstream confirmMessage;
                    confirmMessage << L"هل أنت متأكد من أنك تريد إنهاء العملية التالية؟\n\n"
                        << processName << L" (PID: " << processId << L")";

                    int result = MessageBox(hwnd, confirmMessage.str().c_str(),
                        L"تأكيد إنهاء العملية", MB_YESNO | MB_ICONQUESTION);

                    if (result == IDYES) {
                        if (processManager->terminateProcess(processId)) {
                            MessageBox(hwnd, L"تم إنهاء العملية بنجاح!", L"إنهاء العملية", MB_OK | MB_ICONINFORMATION);
                            FillProcessList();
                        }
                        else {
                            MessageBox(hwnd, L"فشل في إنهاء العملية.", L"خطأ", MB_OK | MB_ICONERROR);
                        }
                    }
                }
                else {
                    MessageBox(hwnd, L"الرجاء اختيار عملية من القائمة أولاً.", L"خطأ", MB_OK | MB_ICONWARNING);
                }
                return 0;
            }
        }
        break;

    case WM_NOTIFY: {
        NMHDR* nmhdr = (NMHDR*)lParam;
        if (nmhdr->hwndFrom == hProcessList) {
            if (nmhdr->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* pnmv = (NMLISTVIEW*)lParam;
                if (pnmv->uNewState & LVIS_SELECTED) {
                    // تم تحديد عملية جديدة، قم بتحديث معلوماتها
                    UpdateProcessInfo();
                }
            }
        }
        break;
    }
    }

    return CallWindowProc(oldTabProc, hwnd, uMsg, wParam, lParam);
}

void ProcessTab::FillProcessList() {
    if (!processManager) return;

    // الحصول على قائمة العمليات الحالية
    processes = processManager->getProcessList();

    // حفظ العملية المحددة حاليًا
    int selectedIndex = ListView_GetNextItem(hProcessList, -1, LVNI_SELECTED);
    DWORD selectedProcessId = 0;
    if (selectedIndex >= 0 && selectedIndex < processes.size()) {
        selectedProcessId = processes[selectedIndex].id;
    }

    // إزالة جميع العناصر من القائمة
    ListView_DeleteAllItems(hProcessList);

    // إضافة العمليات إلى القائمة
    int index = 0;
    for (const auto& process : processes) {
        LVITEM lvi;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = index;
        lvi.iSubItem = 0;

        // تحويل اسم العملية إلى Unicode
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring processName = converter.from_bytes(process.name);
        lvi.pszText = const_cast<LPWSTR>(processName.c_str());
        lvi.lParam = process.id; // تخزين معرف العملية

        ListView_InsertItem(hProcessList, &lvi);

        // إضافة معرف العملية
        std::wstring processId = std::to_wstring(process.id);
        ListView_SetItemText(hProcessList, index, 1, const_cast<LPWSTR>(processId.c_str()));

        // إضافة استخدام الذاكرة (بالميجابايت)
        double memoryMB = process.memoryUsage / (1024.0 * 1024.0);
        std::wstringstream memoryText;
        memoryText << std::fixed << std::setprecision(2) << memoryMB << L" MB";
        std::wstring memoryStr = memoryText.str();
        ListView_SetItemText(hProcessList, index, 2, const_cast<LPWSTR>(memoryStr.c_str()));

        // إضافة استخدام المعالج
        std::wstring cpuUsage = std::to_wstring(static_cast<int>(process.cpuUsage)) + L"%";
        ListView_SetItemText(hProcessList, index, 3, const_cast<LPWSTR>(cpuUsage.c_str()));

        // إضافة حالة العملية
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> statusConverter;
        std::wstring status = statusConverter.from_bytes(process.status);
        ListView_SetItemText(hProcessList, index, 4, const_cast<LPWSTR>(status.c_str()));

        index++;
    }

    // استعادة العملية المحددة سابقًا إن وجدت
    if (selectedProcessId != 0) {
        for (int i = 0; i < ListView_GetItemCount(hProcessList); i++) {
            LVITEM lvi;
            lvi.mask = LVIF_PARAM;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            ListView_GetItem(hProcessList, &lvi);

            if (lvi.lParam == selectedProcessId) {
                ListView_SetItemState(hProcessList, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                break;
            }
        }
    }

    // تحديث معلومات العملية المحددة
    UpdateProcessInfo();
}

void ProcessTab::UpdateProcessInfo() {
    int selectedIndex = ListView_GetNextItem(hProcessList, -1, LVNI_SELECTED);
    if (selectedIndex >= 0 && selectedIndex < processes.size()) {
        // الحصول على معلومات العملية المحددة
        ProcessInfo process = processes[selectedIndex];

        // تنسيق المعلومات
        std::wstringstream infoText;
        infoText << L"معلومات العملية:\r\n\r\n";

        // تحويل اسم العملية إلى Unicode
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring processName = converter.from_bytes(process.name);

        infoText << L"الاسم: " << processName << L"\r\n";
        infoText << L"معرف العملية: " << process.id << L"\r\n";

        // حساب استخدام الذاكرة بوحدات مختلفة
        double memoryKB = process.memoryUsage / 1024.0;
        double memoryMB = memoryKB / 1024.0;
        infoText << L"استخدام الذاكرة: " << std::fixed << std::setprecision(2)
            << memoryMB << L" MB (" << std::setprecision(0) << memoryKB << L" KB)\r\n";

        infoText << L"استخدام المعالج: " << std::fixed << std::setprecision(1) << process.cpuUsage << L"%\r\n";

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> statusConverter;
        std::wstring status = statusConverter.from_bytes(process.status);
        infoText << L"الحالة: " << status << L"\r\n";

        // تحديث نص الملصق
        SetWindowText(hProcessInfoStatic, infoText.str().c_str());
    }
    else {
        // إذا لم يتم اختيار أي عملية
        SetWindowText(hProcessInfoStatic, L"اختر عملية من القائمة لعرض معلوماتها");
    }
}