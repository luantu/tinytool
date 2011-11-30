#pragma once
#include "stdafx.h"

#define ICON_W		16
#define ICON_H		16

#define CHAR_N		4
#define CHAR_W		3
#define CHAR_H		5
#define CHAR_GAP	1
#define LINETOP_0	1
#define LINETOP_1	10
#define GAP_L		1

#define CPU_H(x)	(250 - (x << 1) - (x >> 1))
#define CPU_S		255
#define CPU_V		255

#define CPU_COLOR	(RGB(0xff, 0xff, 0x00))
#define MEM_COLOR	(RGB(0x00, 0xff, 0x00))

class PerformanceIcon
{
private:
	HBITMAP hbmAscii;
	HBITMAP hbmMask;
	HBRUSH	hbrCpuBrush;
	HBRUSH	hbrMemBrush;
	static const RECT rcChar;
	static const RECT rcIcon;
protected:
	void initMask();
	void makeCpuChars(char percentage, char* buff);
	void makeMemoryChars(int dispMemory, char unit, char* buff);
public:
	PerformanceIcon(void);
	~PerformanceIcon(void);

	HICON getIcon(char* pCpuPercentages, int nCpu, int displayMemory, char unit);
};
