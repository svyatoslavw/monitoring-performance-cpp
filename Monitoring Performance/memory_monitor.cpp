#include "memory_monitor.h"

std::wstring MemoryMonitor::GetMemoryData(int& memoryLoad) {
    if (!Initialize()) return L"Failed to initialize WMI";

    IEnumWbemClassObject* enumerator = nullptr;
    std::wostringstream output;

    if (ExecuteQuery(L"SELECT * FROM Win32_OperatingSystem", &enumerator)) {
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        while (enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            VARIANT vtProp;

            pclsObj->Get(L"TotalVisibleMemorySize", 0, &vtProp, 0, 0);
            unsigned long long totalMemory = _wtoi64(vtProp.bstrVal);
            VariantClear(&vtProp);

            pclsObj->Get(L"FreePhysicalMemory", 0, &vtProp, 0, 0);
            unsigned long long freeMemory = _wtoi64(vtProp.bstrVal);
            VariantClear(&vtProp);

            pclsObj->Get(L"Cached", 0, &vtProp, 0, 0);
            unsigned long long cachedMemory = _wtoi64(vtProp.bstrVal);
            VariantClear(&vtProp);

            memoryLoad = static_cast<int>(100 - ((freeMemory * 100) / totalMemory));

            output << L"Розмір оперативної пам'яті: " << totalMemory / 1024 << L" MB\n";
            output << L"Вільна пам'ять: " << freeMemory / 1024 << L" MB\n";
            output << L"Використана пам'ять: " << memoryLoad << L"%\n";
            output << L"Закешована пам'ять: " << cachedMemory / 1024 << L" MB\n";

            pclsObj->Get(L"Status", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_BSTR) {
                output << L"Стан системи: " << vtProp.bstrVal << L"\n";
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }

        enumerator->Release();
    }

    Cleanup();
    return output.str();
}