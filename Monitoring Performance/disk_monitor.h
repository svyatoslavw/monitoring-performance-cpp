#pragma once
#include "wmi.h"

class DiskMonitor : public WmiClient
{
public:
	std::wstring GetDiskData(int& diskLoad);
};