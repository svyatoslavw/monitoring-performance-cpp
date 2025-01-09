#include "ui.h"
#include "Resource.h"
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include "logger.cpp"

#pragma comment(lib, "wbemuuid.lib")

#define WINDOW_CLASS_NAME L"PerformanceMonitor"

Logger logger;

UI::UI(HINSTANCE hInstance) : hInstance(hInstance), mainWindow(nullptr) {}

void StartUpdateInterval(HWND hWnd, UINT_PTR* timerId, UINT interval);
void StopUpdateInterval(HWND hWnd, UINT_PTR timerId);

UI::~UI() {}

bool UI::Initialize() {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, NULL, LoadCursor(nullptr, IDC_ARROW),
                      (HBRUSH)(COLOR_WINDOW + 1), NULL, WINDOW_CLASS_NAME, NULL };

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    mainWindow = CreateWindow(WINDOW_CLASS_NAME, L"Моніторинг продуктивності", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 500, 250, NULL, NULL, hInstance, this);

    if (!mainWindow) {
        return false;
    }

    ShowWindow(mainWindow, SW_SHOW);
    UpdateWindow(mainWindow);
    return true;
}

void UI::Run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK UI::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    UI* ui = nullptr;

    if (message == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        ui = (UI*)cs->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ui);
    }
    else {
        ui = (UI*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }

    if (ui) {
        switch (message) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
            case IDM_CPUMODEL:
                ui->ShowCPUDialog(hWnd);
                break;
            case IDM_MEMORYMODEL:
                ui->ShowMemoryDialog(hWnd);
                break;
            case IDM_DISKMODEL:
                ui->ShowDiskDialog(hWnd);
                break;
			case IDM_NETWORKMODEL:
				ui->ShowNetworkDialog(hWnd);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_CREATE: 
            CreateWindowEx(0, L"BUTTON", L"Процесор",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                10, 10, 140, 30, hWnd, (HMENU)IDM_CPUMODEL, ui->hInstance, NULL);
            CreateWindowEx(0, L"BUTTON", L"Оперативна пам'ять",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                10, 50, 140, 30, hWnd, (HMENU)IDM_MEMORYMODEL, ui->hInstance, NULL);
            CreateWindowEx(0, L"BUTTON", L"Внутрішня пам'ять",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                10, 90, 140, 30, hWnd, (HMENU)IDM_DISKMODEL, ui->hInstance, NULL);
            CreateWindowEx(0, L"BUTTON", L"Мережа",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                10, 130, 140, 30, hWnd, (HMENU)IDM_NETWORKMODEL, ui->hInstance, NULL);
			break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    else {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

INT_PTR UI::ShowCPUDialog(HWND hParent) {
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CPUMODEL), hParent, CPUProc, (LPARAM)this);
}

INT_PTR UI::ShowMemoryDialog(HWND hParent) {
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MEMORYMODEL), hParent, MemoryProc, (LPARAM)this);
}

INT_PTR UI::ShowDiskDialog(HWND hParent) {
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DISKMODEL), hParent, DiskProc, (LPARAM)this);
}

INT_PTR UI::ShowNetworkDialog(HWND hParent) {
	return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_NETWORKMODEL), hParent, NetworkProc, (LPARAM)this);
}

INT_PTR CALLBACK UI::CPUProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static std::wstring cpuPerformanceData;
    static std::vector<int> cpuLoadHistory(100, 0);
    static int cpuLoad = 0;
    static UINT_PTR timerId = 0;

    switch (message) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        {
            UI* ui = (UI*)lParam;
            if (ui) {
                cpuPerformanceData = ui->cpuMonitor.GetCpuData(cpuLoad);
                StartUpdateInterval(hDlg, &timerId, 100);
            }
            RECT rect;
            GetClientRect(hDlg, &rect);

            int buttonWidth = 120;
            int buttonHeight = 30;
            int buttonSpacing = 10;
            int buttonY = rect.bottom - buttonHeight - 70;

            CreateWindow(L"BUTTON", L"Почати лог", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10, buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)11, GetModuleHandle(NULL), NULL);
            CreateWindow(L"BUTTON", L"Завершити лог", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10 + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)22, GetModuleHandle(NULL), NULL);
            CreateWindow(L"BUTTON", L"Відкрити файл", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10 + 2 * (buttonWidth + buttonSpacing), buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)33, GetModuleHandle(NULL), NULL);
        }
        return (INT_PTR)TRUE;
    case WM_TIMER:
        cpuPerformanceData = ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->cpuMonitor.GetCpuData(cpuLoad);
        cpuLoadHistory.erase(cpuLoadHistory.begin());
        cpuLoadHistory.push_back(cpuLoad);
        InvalidateRect(hDlg, NULL, TRUE);
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        RECT rect;
        GetClientRect(hDlg, &rect);

        RECT cpuTextRect = { rect.left, 0, rect.right / 2, rect.bottom };
        DrawText(hdc, cpuPerformanceData.c_str(), -1, &cpuTextRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

        RECT cpuGraphRect = { rect.right / 2 + 10, 0, rect.right, 100 };
        ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->DrawGraph(hdc, cpuGraphRect, cpuLoadHistory);
        EndPaint(hDlg, &ps);
    }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 11:
            logger.StartLogging();
            break;
        case 22:
            logger.StopLogging(hDlg, cpuLoadHistory, L"cpu_log.txt");
            break;
        case 33:
            ShellExecute(NULL, L"open", L"cpu_log.txt", NULL, NULL, SW_SHOWNORMAL);
        case IDOK:
        case IDCANCEL:
            logger.StopLogging(hDlg, cpuLoadHistory, L"cpu_log.txt");
            StopUpdateInterval(hDlg, timerId);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK UI::MemoryProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static std::wstring memoryPerformanceData;
    static std::vector<int> memoryLoadHistory(100, 0);
    static int memoryLoad = 0;
    static UINT_PTR timerId = 0;

    switch (message) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        {
            UI* ui = (UI*)lParam;
            if (ui) {
                memoryPerformanceData = ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->memoryMonitor.GetMemoryData(memoryLoad);
                StartUpdateInterval(hDlg, &timerId, 1000);
            }
            RECT rect;
            GetClientRect(hDlg, &rect);

            int buttonWidth = 120;
            int buttonHeight = 30;
            int buttonSpacing = 10;
            int buttonY = rect.bottom - buttonHeight - 70;

            CreateWindow(L"BUTTON", L"Почати лог", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10, buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)11, GetModuleHandle(NULL), NULL);
            CreateWindow(L"BUTTON", L"Завершити лог", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10 + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)22, GetModuleHandle(NULL), NULL);
            CreateWindow(L"BUTTON", L"Відкрити файл", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10 + 2 * (buttonWidth + buttonSpacing), buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)33, GetModuleHandle(NULL), NULL);
        }
        return (INT_PTR)TRUE;

    case WM_TIMER:
        memoryPerformanceData = ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->memoryMonitor.GetMemoryData(memoryLoad);
        memoryLoadHistory.erase(memoryLoadHistory.begin());
        memoryLoadHistory.push_back(memoryLoad);
        InvalidateRect(hDlg, NULL, TRUE);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        RECT rect;
        GetClientRect(hDlg, &rect);

        RECT memoryTextRect = { rect.left, 0, rect.right / 2, 100 };
        DrawText(hdc, memoryPerformanceData.c_str(), -1, &memoryTextRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

        RECT memoryGraphRect = { rect.right / 2 + 10, 0, rect.right, 100 };
        ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->DrawGraph(hdc, memoryGraphRect, memoryLoadHistory);
        EndPaint(hDlg, &ps);
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 11:
            logger.StartLogging();
            break;
        case 22:
            logger.StopLogging(hDlg, memoryLoadHistory, L"memory_log.txt");
            break;
        case 33:
            ShellExecute(NULL, L"open", L"memory_log.txt", NULL, NULL, SW_SHOWNORMAL);
        case IDOK:
        case IDCANCEL:
            logger.StopLogging(hDlg, memoryLoadHistory, L"memory_log.txt");
            StopUpdateInterval(hDlg, timerId);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
     break;
    }
    return (INT_PTR)FALSE;
}


INT_PTR CALLBACK UI::DiskProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static std::wstring diskPerformanceData;
    static std::vector<int> diskLoadHistory(100, 0);
    static int diskLoad = 0;
    static UINT_PTR timerId = 0;

    switch (message) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        {
            UI* ui = (UI*)lParam;
            if (ui) {
                diskPerformanceData = ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->diskMonitor.GetDiskData(diskLoad);
                StartUpdateInterval(hDlg, &timerId, 100);
            }
            RECT rect;
            GetClientRect(hDlg, &rect);

            int buttonWidth = 120;
            int buttonHeight = 30;
            int buttonSpacing = 10;
            int buttonY = rect.bottom - buttonHeight - 70;

            CreateWindow(L"BUTTON", L"Почати лог", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10, buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)11, GetModuleHandle(NULL), NULL);
            CreateWindow(L"BUTTON", L"Завершити лог", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10 + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)22, GetModuleHandle(NULL), NULL);
            CreateWindow(L"BUTTON", L"Відкрити файл", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10 + 2 * (buttonWidth + buttonSpacing), buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)33, GetModuleHandle(NULL), NULL);
        }
        return (INT_PTR)TRUE;

    case WM_TIMER:
        diskPerformanceData = ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->diskMonitor.GetDiskData(diskLoad);
        diskLoadHistory.erase(diskLoadHistory.begin());
        diskLoadHistory.push_back(diskLoad);
        InvalidateRect(hDlg, NULL, TRUE);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        RECT rect;
        GetClientRect(hDlg, &rect);
        RECT diskTextRect = { rect.left, 0, rect.right / 2, rect.bottom };
        DrawText(hdc, diskPerformanceData.c_str(), -1, &diskTextRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

        RECT diskGraphRect = { rect.right / 2 + 10, 0, rect.right, 100 };
        ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->DrawGraph(hdc, diskGraphRect, diskLoadHistory);
        EndPaint(hDlg, &ps);
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 11:
            logger.StartLogging();
            break;
        case 22:
            logger.StopLogging(hDlg, diskLoadHistory, L"disk_log.txt");
            break;
        case 33:
            ShellExecute(NULL, L"open", L"disk_log.txt", NULL, NULL, SW_SHOWNORMAL);
        case IDOK:
        case IDCANCEL:
            logger.StopLogging(hDlg, diskLoadHistory, L"disk_log.txt");
            StopUpdateInterval(hDlg, timerId);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK UI::NetworkProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static std::wstring networkPerformanceData;
    static std::vector<int> networkLoadHistory(100, 0);
    static int networkLoad = 0;
    static UINT_PTR timerId = 0;

    switch (message) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        {
            UI* ui = (UI*)lParam;
            if (ui) {
                networkPerformanceData = ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->networkMonitor.GetNetworkData(networkLoad);
                StartUpdateInterval(hDlg, &timerId, 100);
            }
            RECT rect;
            GetClientRect(hDlg, &rect);

            int buttonWidth = 120;
            int buttonHeight = 30;
            int buttonSpacing = 10;
            int buttonY = rect.bottom - buttonHeight - 70;

            CreateWindow(L"BUTTON", L"Почати лог", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10, buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)11, GetModuleHandle(NULL), NULL);
            CreateWindow(L"BUTTON", L"Завершити лог", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10 + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)22, GetModuleHandle(NULL), NULL);
            CreateWindow(L"BUTTON", L"Відкрити файл", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10 + 2 * (buttonWidth + buttonSpacing), buttonY, buttonWidth, buttonHeight, hDlg, (HMENU)33, GetModuleHandle(NULL), NULL);
        }
        return (INT_PTR)TRUE;
    case WM_TIMER:
        networkPerformanceData = ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->networkMonitor.GetNetworkData(networkLoad);
        networkLoadHistory.erase(networkLoadHistory.begin());
        networkLoadHistory.push_back(networkLoad);
        InvalidateRect(hDlg, NULL, TRUE);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        RECT rect;
        GetClientRect(hDlg, &rect);
        RECT networkTextRect = { rect.left, 0, rect.right / 2, 100 };
        DrawText(hdc, networkPerformanceData.c_str(), -1, &networkTextRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

        RECT networkGraphRect = { rect.right / 2 + 10, 0, rect.right, 100 };
        ((UI*)GetWindowLongPtr(hDlg, GWLP_USERDATA))->DrawGraph(hdc, networkGraphRect, networkLoadHistory);
        EndPaint(hDlg, &ps);
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 11:
            logger.StartLogging();
            break;
        case 22:
            logger.StopLogging(hDlg, networkLoadHistory, L"network_log.txt");
            break;
        case 33:
            ShellExecute(NULL, L"open", L"network_log.txt", NULL, NULL, SW_SHOWNORMAL);
        case IDOK:
        case IDCANCEL:
            logger.StopLogging(hDlg, networkLoadHistory, L"network_log.txt");
            StopUpdateInterval(hDlg, timerId);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void UI::DrawGraph(HDC hdc, RECT rect, const std::vector<int>& data) {
    FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int dataCount = static_cast<int>(data.size());
    if (dataCount < 2) return;

    int maxLoad = 100;
    int stepX = width / dataCount;

    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    for (int i = 1; i < dataCount; ++i) {
        int x1 = rect.left + (i - 1) * stepX;
        int y1 = rect.bottom - (data[i - 1] * height / maxLoad);

        int x2 = rect.left + i * stepX;
        int y2 = rect.bottom - (data[i] * height / maxLoad);

        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void StartUpdateInterval(HWND hWnd, UINT_PTR* timerId, UINT interval)
{
    *timerId = SetTimer(hWnd, 1, interval, NULL);
}

void StopUpdateInterval(HWND hWnd, UINT_PTR timerId)
{
    KillTimer(hWnd, timerId);
}