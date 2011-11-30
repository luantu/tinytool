#pragma once

#include "stdafx.h"
#include "pdh.h"
#include "PerformanceIcon.h"

class PerformanceMonitor
{
private:
	HQUERY hQuery;
	HCOUNTER* phCounters;
	BOOL bPdhSuccess;
	int cpuCount;
	char* prevPercentages;
	char* currPercentages;

	ULONGLONG prevMemoryBytes;

	ULONGLONG currMemoryBytes;
	unsigned int dispMemory;
	unsigned char unitIndex;

	static const TCHAR* UNITS;

	PerformanceIcon pi;
public:
	PerformanceMonitor(void);
	~PerformanceMonitor(void);

	BOOL isPdhSuccess() {
		return bPdhSuccess;
	}

	ULONGLONG getMemoryBytes();
	BOOL retrieveCpuPercentage();

	HICON getIcon();
	int cpyTip(TCHAR* tip, size_t len);

	BOOL next();
};
