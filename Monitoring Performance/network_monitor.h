#pragma once
#include "wmi.h"

class NetworkMonitor : public WmiClient
{
public:
	std::wstring GetNetworkData(int& gpuLoad);
};