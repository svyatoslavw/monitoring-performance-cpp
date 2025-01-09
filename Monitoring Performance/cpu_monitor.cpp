#include "cpu_monitor.h"

std::wstring CpuMonitor::GetCpuData(int& cpuLoad) {
    if (!Initialize()) return L"Failed to initialize WMI";

    IEnumWbemClassObject* enumerator = nullptr;
    std::wostringstream output;

    if (ExecuteQuery(L"SELECT * FROM Win32_Processor", &enumerator)) {
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        while (enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            VARIANT vtProp;

            pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
            output << L"Назва процесору: " << vtProp.bstrVal << L"\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"LoadPercentage", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_I4) {
                cpuLoad = vtProp.intVal;
                output << L"Використання процесору: " << cpuLoad << L"%\n";
            }
            VariantClear(&vtProp);


            pclsObj->Get(L"NumberOfCores", 0, &vtProp, 0, 0);
            output << L"Кількість фізичних ядер: " << vtProp.uintVal << L"\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"NumberOfLogicalProcessors", 0, &vtProp, 0, 0);
            output << L"Кількість процесорів: " << vtProp.uintVal << L"\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"MaxClockSpeed", 0, &vtProp, 0, 0);
            output << L"Максимальна частота: " << vtProp.uintVal << L" MHz\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"CurrentClockSpeed", 0, &vtProp, 0, 0);
            output << L"Поточна частота: " << vtProp.uintVal << L" MHz\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"Architecture", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_I4) {
                switch (vtProp.intVal) {
                case 0: output << L"Архітектура: x86\n"; break;
                case 1: output << L"Архітектура: MIPS\n"; break;
                case 2: output << L"Архітектура: Alpha\n"; break;
                case 3: output << L"Архітектура: PowerPC\n"; break;
                case 6: output << L"Архітектура: IA64\n"; break;
                case 9: output << L"Архітектура: x64\n"; break;
                default: output << L"Архітектура: Невідома\n"; break;
                }
            }
            VariantClear(&vtProp);

            pclsObj->Get(L"DataWidth", 0, &vtProp, 0, 0);
            output << L"Розрядність: " << vtProp.uintVal << L" біт\n";
            VariantClear(&vtProp);

            pclsObj->Release();
        }

        enumerator->Release();
    }

    if (ExecuteQuery(L"SELECT * FROM Win32_PerfRawData_PerfOS_System", &enumerator)) {
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        while (enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            VARIANT vtProp;

            pclsObj->Get(L"Processes", 0, &vtProp, 0, 0);
            output << L"Кількість процесів: " << vtProp.uintVal << L"\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"Threads", 0, &vtProp, 0, 0);
            output << L"Кількість потоків: " << vtProp.uintVal << L"\n";
            VariantClear(&vtProp);
        }
        enumerator->Release();
    }

    Cleanup();
    return output.str();
}