#pragma once
#include "wmi.h"

class MemoryMonitor : public WmiClient {
public:
    std::wstring GetMemoryData(int& memoryLoad);
};