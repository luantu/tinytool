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

	unsigned int prevDispMemory;
	unsigned char prevUnitIndex;

	unsigned int dispMemory;
	unsigned char unitIndex;

	unsigned int totalDispMemory;
	unsigned char totalUnitIndex;

	static const TCHAR* UNITS;

	PerformanceIcon* ppi;

protected:
	ULONGLONG getTotalMemoryBytes();
	ULONGLONG getMemoryBytes();
	BOOL retrieveCpuPercentage();
	unsigned int getDispMemory(ULONGLONG mem, unsigned char* ui);
public:
	PerformanceMonitor(void);
	~PerformanceMonitor(void);

	BOOL isPdhSuccess() {
		return bPdhSuccess;
	}

	void setColors(COLORREF cpuColor, COLORREF memColor);

	HICON getIcon();
	int cpyTip(TCHAR* tip, size_t len);

	BOOL next();
};
