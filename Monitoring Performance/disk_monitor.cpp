#include "disk_monitor.h"
#include <chrono>
#include <thread>
#include <sstream>

std::wstring DiskMonitor::GetDiskData(int& diskLoad) {
    if (!Initialize()) return L"Failed to initialize WMI";

    IEnumWbemClassObject* enumerator = nullptr;
    std::wostringstream output;

    if (ExecuteQuery(L"SELECT Name, Size, FreeSpace FROM Win32_LogicalDisk WHERE DriveType=3", &enumerator)) {
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;
        unsigned long long totalDiskSpace = 0;
        unsigned long long freeDiskSpace = 0;

        while (enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            VARIANT vtProp;

            pclsObj->Get(L"Size", 0, &vtProp, 0, 0);
            totalDiskSpace += _wtoi64(vtProp.bstrVal);
            VariantClear(&vtProp);

            pclsObj->Get(L"FreeSpace", 0, &vtProp, 0, 0);
            freeDiskSpace += _wtoi64(vtProp.bstrVal);
            VariantClear(&vtProp);

               
            pclsObj->Release();
        }

        enumerator->Release();
        if (totalDiskSpace > 0) {
            diskLoad = static_cast<int>(100 - ((freeDiskSpace * 100) / totalDiskSpace));
        }

        unsigned long long usedDiskSpace = totalDiskSpace - freeDiskSpace;
        output << L"Диски завантажені на: " << diskLoad << L"%\n";
        output << L"Весь простір дисків: " << totalDiskSpace / (1024 * 1024 * 1024) << L" GB\n";
        output << L"Використаний простір дисків: " << usedDiskSpace / (1024 * 1024 * 1024) << L" GB\n";
        output << L"Вільний простір дисків: " << freeDiskSpace / (1024 * 1024 * 1024) << L" GB\n\n";
    }

    Cleanup();

    return output.str();
}
