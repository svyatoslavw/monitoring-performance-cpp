#pragma once
#include "cpu_monitor.h"
#include "memory_monitor.h"
#include <string>
#include <vector>
#include <windows.h>
#include "disk_monitor.h"
#include "network_monitor.h"

class UI {
public:
    UI(HINSTANCE hInstance);
    ~UI();

    bool Initialize();
    void Run();

    // Dialog handlers
    INT_PTR ShowCPUDialog(HWND hParent);
    INT_PTR ShowMemoryDialog(HWND hParent);
    INT_PTR ShowDiskDialog(HWND hParent);
	INT_PTR ShowNetworkDialog(HWND hParent);

private:
    HINSTANCE hInstance;
    HWND mainWindow;
    CpuMonitor cpuMonitor;
    MemoryMonitor memoryMonitor;
	DiskMonitor diskMonitor;
    NetworkMonitor networkMonitor;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK CPUProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK MemoryProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK DiskProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK NetworkProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    void DrawGraph(HDC hdc, RECT rect, const std::vector<int>& data);
};
