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
            output << L"����� ���������: " << vtProp.bstrVal << L"\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"LoadPercentage", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_I4) {
                cpuLoad = vtProp.intVal;
                output << L"������������ ���������: " << cpuLoad << L"%\n";
            }
            VariantClear(&vtProp);


            pclsObj->Get(L"NumberOfCores", 0, &vtProp, 0, 0);
            output << L"ʳ������ �������� ����: " << vtProp.uintVal << L"\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"NumberOfLogicalProcessors", 0, &vtProp, 0, 0);
            output << L"ʳ������ ���������: " << vtProp.uintVal << L"\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"MaxClockSpeed", 0, &vtProp, 0, 0);
            output << L"����������� �������: " << vtProp.uintVal << L" MHz\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"CurrentClockSpeed", 0, &vtProp, 0, 0);
            output << L"������� �������: " << vtProp.uintVal << L" MHz\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"Architecture", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_I4) {
                switch (vtProp.intVal) {
                case 0: output << L"�����������: x86\n"; break;
                case 1: output << L"�����������: MIPS\n"; break;
                case 2: output << L"�����������: Alpha\n"; break;
                case 3: output << L"�����������: PowerPC\n"; break;
                case 6: output << L"�����������: IA64\n"; break;
                case 9: output << L"�����������: x64\n"; break;
                default: output << L"�����������: �������\n"; break;
                }
            }
            VariantClear(&vtProp);

            pclsObj->Get(L"DataWidth", 0, &vtProp, 0, 0);
            output << L"����������: " << vtProp.uintVal << L" ��\n";
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
            output << L"ʳ������ �������: " << vtProp.uintVal << L"\n";
            VariantClear(&vtProp);

            pclsObj->Get(L"Threads", 0, &vtProp, 0, 0);
            output << L"ʳ������ ������: " << vtProp.uintVal << L"\n";
            VariantClear(&vtProp);
        }
        enumerator->Release();
    }

    Cleanup();
    return output.str();
}