#include "wmi.h"
#include <Wbemidl.h>
#include <comdef.h>

#pragma comment(lib, "wbemuuid.lib")

WmiClient::WmiClient() : pLoc(nullptr), pSvc(nullptr), hres(S_OK) {}

WmiClient::~WmiClient() {
    Cleanup();
}

bool WmiClient::Initialize() {
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        return false;
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) {
        CoUninitialize();
        return false;
    }

    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) {
        CoUninitialize();
        return false;
    }

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        Cleanup();
        return false;
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);
    if (FAILED(hres)) {
        Cleanup();
        return false;
    }

    return true;
}

void WmiClient::Cleanup() {
    if (pSvc) {
        pSvc->Release();
        pSvc = nullptr; 
    }
    if (pLoc) {
        pLoc->Release();
        pLoc = nullptr;
    }

    CoUninitialize();
}

bool WmiClient::ExecuteQuery(const std::wstring& query, IEnumWbemClassObject** enumerator) {
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        enumerator);

    return SUCCEEDED(hres);
}
