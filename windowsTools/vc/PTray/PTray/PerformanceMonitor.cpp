#include "PerformanceMonitor.h"
#include <stdio.h>

const TCHAR* PerformanceMonitor::UNITS = _T("BKMGTPEZY");

PerformanceMonitor::PerformanceMonitor(void)
{
	this->dispMemory = 0;
	this->unitIndex = 0;
	this->totalDispMemory = 0;
	this->totalUnitIndex = 0;
	this->ppi = NULL;

	SYSTEM_INFO si = {0};
	GetSystemInfo(&si);
	this->cpuCount = si.dwNumberOfProcessors;
	this->phCounters = (HCOUNTER*)calloc(this->cpuCount + 1, sizeof(HCOUNTER));
	this->prevPercentages = (char*) calloc(this->cpuCount + 1, sizeof(char));
	this->currPercentages = (char*) calloc(this->cpuCount + 1, sizeof(char));

	this->bPdhSuccess = PdhOpenQuery(NULL, 0, &this->hQuery) == ERROR_SUCCESS;
	PdhAddCounter(hQuery, _T("\\Processor(_Total)\\% Processor Time"), 0, phCounters);
	TCHAR* format = _T("\\Processor(%d)\\%% Processor Time");
	for (int i = 0; i < this->cpuCount; i++) {
		TCHAR name[32] = {0};
		_stprintf_p(name, sizeof(name), format, i);
		PdhAddCounter(hQuery, name, 0, phCounters + i + 1);
	}
	PdhCollectQueryData(hQuery);

	this->totalDispMemory = this->getDispMemory(this->getTotalMemoryBytes(), &this->totalUnitIndex);
}

PerformanceMonitor::~PerformanceMonitor(void)
{
	PdhCloseQuery(hQuery);
	if (this->ppi) {
		delete this->ppi;
		this->ppi = NULL;
	}
	free(this->phCounters);
	free(this->prevPercentages);
	free(this->currPercentages);
}

void PerformanceMonitor::setColors(COLORREF cpuColor, COLORREF memColor)
{
	if (this->ppi) {
		delete this->ppi;
		this->ppi = NULL;
	}
	this->ppi = new PerformanceIcon(cpuColor, memColor);
}

ULONGLONG PerformanceMonitor::getMemoryBytes()
{
	MEMORYSTATUSEX mse = {0};
	mse.dwLength = sizeof(mse);
	::GlobalMemoryStatusEx(&mse);
	return mse.ullAvailPhys;
}

ULONGLONG PerformanceMonitor::getTotalMemoryBytes()
{
	MEMORYSTATUSEX mse = {0};
	mse.dwLength = sizeof(mse);
	::GlobalMemoryStatusEx(&mse);
	return mse.ullTotalPhys;
}

BOOL PerformanceMonitor::retrieveCpuPercentage()
{
	if (!this->bPdhSuccess) {
		return FALSE;
	}
	PDH_FMT_COUNTERVALUE fmtValue;
	PDH_STATUS status = PdhCollectQueryData(hQuery);
	BOOL ret = status == ERROR_SUCCESS;
	if (ret) {
		for (int i = 0; i <= this->cpuCount; i++) {
			status = PdhGetFormattedCounterValue(phCounters[i], PDH_FMT_LONG, NULL, &fmtValue);
			ret = ret && status == ERROR_SUCCESS;
			this->currPercentages[i] = (char)(fmtValue.longValue);
		}
	}
	return status == ERROR_SUCCESS;
}

unsigned int PerformanceMonitor::getDispMemory(ULONGLONG mem, unsigned char *ui)
{
	if (mem >= 1000) {
		*ui = 1;
		while ((mem >> 10) >= 1000) {
			mem >>= 10;
			(*ui)++;
		}
	} else {
		*ui = 0;
	}
	return (unsigned int)mem;
}

BOOL PerformanceMonitor::next()
{
	this->retrieveCpuPercentage();
	this->dispMemory = this->getDispMemory(this->getMemoryBytes(), &this->unitIndex);

	BOOL ret = !(
		(memcmp(this->currPercentages, this->prevPercentages, this->cpuCount + 1) == 0) && 
		(this->dispMemory == this->prevDispMemory) &&
		(this->unitIndex == this->prevUnitIndex));

	memcpy(this->prevPercentages, this->currPercentages, this->cpuCount + 1);
	this->prevDispMemory = this->dispMemory;
	this->prevUnitIndex = this->unitIndex;
	return ret;
}

HICON PerformanceMonitor::getIcon()
{
	if (!this->ppi) {
		this->ppi = new PerformanceIcon();
	}
	return ppi->getIcon(this->currPercentages, this->cpuCount, this->dispMemory, (char)PerformanceMonitor::UNITS[this->unitIndex]);
}

int PerformanceMonitor::cpyTip(TCHAR *tip, size_t len)
{
	TCHAR mem[10] = {0};
	if ((this->dispMemory >> 10) < 10) {
		_stprintf_p(mem, sizeof(mem)/sizeof(TCHAR), _T("%d.%d%c"), this->dispMemory >> 10, ((this->dispMemory & 0x3ff) * 10) >> 10, PerformanceMonitor::UNITS[this->unitIndex]);
	} else {
		_stprintf_p(mem, sizeof(mem)/sizeof(TCHAR), _T("%3d%c"), this->dispMemory >> 10, PerformanceMonitor::UNITS[this->unitIndex]);
	}
	if ((this->totalDispMemory >> 10) < 10) {
		_stprintf_p(mem + 4, sizeof(mem)/sizeof(TCHAR), _T("/%d.%d%c"), this->totalDispMemory >> 10, ((this->totalDispMemory & 0x3ff) * 10) >> 10, PerformanceMonitor::UNITS[this->totalUnitIndex]);
	} else {
		_stprintf_p(mem + 4, sizeof(mem)/sizeof(TCHAR), _T("/%3d%c"), this->totalDispMemory >> 10, PerformanceMonitor::UNITS[this->totalUnitIndex]);
	}
	static const char CPUBYTE = 5;
	size_t l = this->cpuCount * CPUBYTE;
	TCHAR* sCPUs = (TCHAR*) calloc(l + 1, sizeof(TCHAR));
	for (int i = 0; i < this->cpuCount; i++) {
		TCHAR buff[CPUBYTE] = {0};
		_itot_s(this->currPercentages[i+1], buff, CPUBYTE, 10);
		_tcscat_s(sCPUs, l, buff);
		if (i < this->cpuCount - 1) {
			_tcscat_s(sCPUs, l, _T("|"));
		}
	}
	static TCHAR* format = _T("CPU: %d%% [%s]\nMemory: %s");
	int ret = _stprintf_p(tip, len, format, this->currPercentages[0], sCPUs, mem);
	free(sCPUs);
	return ret;
}