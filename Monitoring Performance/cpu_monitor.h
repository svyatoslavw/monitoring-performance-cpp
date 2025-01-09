#pragma once
#include "wmi.h"

class CpuMonitor : public WmiClient {
public:
    std::wstring GetCpuData(int& cpuLoad);
};