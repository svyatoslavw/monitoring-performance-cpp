#pragma once
// WmiClient.h
#pragma once
#include <string>
#include <Wbemidl.h>
#include <comdef.h>
#include <sstream>

#pragma comment(lib, "wbemuuid.lib")

class WmiClient {
public:
    WmiClient();
    ~WmiClient();

    bool Initialize();
    void Cleanup();

protected:
    IWbemLocator* pLoc;
    IWbemServices* pSvc;
    HRESULT hres;

    bool ExecuteQuery(const std::wstring& query, IEnumWbemClassObject** enumerator);
};
