#include "network_monitor.h"
#include <chrono>
#include <sstream>

std::wstring NetworkMonitor::GetNetworkData(int& networkLoad) {
	if (!Initialize()) return L"Failed to initialize WMI";

	IEnumWbemClassObject* enumerator = nullptr;
	std::wostringstream output;

	if (ExecuteQuery(L"SELECT Name, BytesReceivedPerSec, BytesSentPerSec FROM Win32_PerfFormattedData_Tcpip_NetworkInterface WHERE Name LIKE '%Wi-Fi%'", &enumerator)) {
		IWbemClassObject* pclsObj = nullptr;
		ULONG uReturn = 0;
		ULONGLONG prevBytesReceived = 0, prevBytesSent = 0;
		auto prevTime = std::chrono::high_resolution_clock::now();

		while (enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
			if (0 == uReturn) break;

            VARIANT vtProp;

            pclsObj->Get(L"BytesReceivedPerSec", 0, &vtProp, 0, 0);
            ULONGLONG bytesReceived = vtProp.uintVal;
            VariantClear(&vtProp);

            pclsObj->Get(L"BytesSentPerSec", 0, &vtProp, 0, 0);
            ULONGLONG bytesSent = vtProp.uintVal;
            VariantClear(&vtProp);

            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedTime = currentTime - prevTime;

            ULONGLONG deltaReceived = bytesReceived - prevBytesReceived;
            ULONGLONG deltaSent = bytesSent - prevBytesSent;

            double speedReceived = (deltaReceived / elapsedTime.count()) / (1024 * 1024);
            double speedSent = (deltaSent / elapsedTime.count()) / (1024 * 1024);

            output << L"Ўвидк≥сть отримки: " << speedReceived << L" MB/s\n";
            output << L"Ўвидк≥сть в≥дправки: " << speedSent << L" MB/s\n";

            prevBytesReceived = bytesReceived;
            prevBytesSent = bytesSent;
            prevTime = currentTime;

            networkLoad = static_cast<int>((speedReceived + speedSent) * 100);

			pclsObj->Release();
		}

		enumerator->Release();
	}

    Cleanup();
	return output.str();

}