#include "PerformanceIcon.h"
#include "resource.h"
#include <stdio.h>

const RECT PerformanceIcon::rcChar = {0, 0, CHAR_W, CHAR_H};
const RECT PerformanceIcon::rcIcon = {0, 0, ICON_W, ICON_H};

static void HSV2RGB(int h, int s, int v, COLORREF* rgb);
PerformanceIcon::PerformanceIcon(void)
{
	this->hbmAscii = ::LoadBitmap(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ASCII));
	this->hbrCpuBrush = ::CreateSolidBrush(CPU_COLOR);
	this->hbrMemBrush = ::CreateSolidBrush(MEM_COLOR);
	this->initMask();
}

void PerformanceIcon::initMask()
{
	HDC hdc = ::GetDC(NULL);
	HDC hdcMask = ::CreateCompatibleDC(hdc);
	this->hbmMask = ::CreateCompatibleBitmap(hdc, ICON_W, ICON_H);
	HBITMAP hbmOldMask = (HBITMAP)::SelectObject(hdc, hbmMask);
	::FillRect(hdcMask, &rcIcon, GetStockBrush(BLACK_BRUSH));
	::SelectObject(hdcMask, hbmOldMask);
	::DeleteDC(hdcMask);
	::ReleaseDC(NULL, hdc);
}

PerformanceIcon::~PerformanceIcon(void)
{
	if (this->hbmAscii) {
		::DeleteObject(this->hbmAscii);
		this->hbmAscii = NULL;
	}
	if (this->hbmMask) {
		::DeleteObject(this->hbmMask);
		this->hbmMask = NULL;
	}
	if (this->hbrCpuBrush) {
		::DeleteObject(this->hbrCpuBrush);
		this->hbrCpuBrush = NULL;
	}
	if (this->hbrMemBrush) {
		::DeleteObject(this->hbrMemBrush);
		this->hbrMemBrush = NULL;
	}
}

HICON PerformanceIcon::getIcon(char *pCpuPercentages, int nCpu, int displayMemory, char unit)
{
	static unsigned __int32 linePos[CPU_MAX >> 1] = {
		//76543210
		0x00000007, // 1
		0x000000e0, // 2
		0x00000e70, // 3
		0x0000e950, // 4
		0x000ea740, // 5
		0x00eb8630, // 6
		0x0eca7420, // 7
		0xeca86420, // 8
	};
	//static unsigned char ptCpuLights[] = {
	//	0x17, 0xd7, 
	//	0x28, 0xe8,
	//	0x57, 0x97, 
	//	0x68, 0xa8, 
	//	0x37, 0xb7, 
	//	0x48, 0xc8, 
	//	0x77, 0x88, 
	//	0x11, 0x15,
	//};
	char buff[CHAR_N];

	HDC hdc = ::GetDC(NULL);
	HDC hdcBuff = ::CreateCompatibleDC(hdc);
	HDC hdcChar = ::CreateCompatibleDC(hdc);

	HBITMAP hbmBuff = ::CreateCompatibleBitmap(hdc, ICON_W, ICON_H);
	HBITMAP hbmChar = ::CreateCompatibleBitmap(hdc, CHAR_W, CHAR_H);

	HBITMAP hbmOldBuff = (HBITMAP)::SelectObject(hdcBuff, hbmBuff);
	HBITMAP hbmOldChar = (HBITMAP)::SelectObject(hdcChar, hbmChar);

	::FillRect(hdcBuff, &rcIcon, GetStockBrush(BLACK_BRUSH));

	// CPU average percentage
	::FillRect(hdcChar, &rcChar, this->hbrCpuBrush);
	this->makeCpuChars(pCpuPercentages[0], buff);
	for (int i = 0; i < CHAR_N; i++) {
		if (buff[i] != ' ') {
			int x = GAP_L + (CHAR_W + CHAR_GAP) * i;
			int y = LINETOP_0;
			::MaskBlt(hdcBuff, x, y, CHAR_W, CHAR_H, hdcChar, 0, 0, this->hbmAscii, 0, CHAR_H * (buff[i] - ' '), MAKEROP4(BLACKNESS, SRCPAINT));
		}
	}

	// Memory available
	::FillRect(hdcChar, &rcChar, this->hbrMemBrush);
	this->makeMemoryChars(displayMemory, unit, buff);
	for (int i = 0; i < CHAR_N; i++) {
		if (buff[i] != ' ') {
			int x = GAP_L + (CHAR_W + CHAR_GAP) * i;
			int y = LINETOP_1;
			::MaskBlt(hdcBuff, x, y, CHAR_W, CHAR_H, hdcChar, 0, 0, this->hbmAscii, 0, CHAR_H * (buff[i] - ' '), MAKEROP4(BLACKNESS, SRCPAINT));
		}
	}

	// CPU lights for every core
	if (nCpu > CPU_MAX) {
		nCpu = CPU_MAX;
	}
	int half = nCpu - (nCpu >> 1);
	unsigned __int32 lp1 = linePos[half-1];
	unsigned __int32 lp2 = linePos[nCpu - half - 1];
	for (int i = 0; i < nCpu; i++) {
		int p = pCpuPercentages[i + 1];
		COLORREF color = {0};
		HSV2RGB(CPU_H(p), CPU_S, CPU_V, &color);
		unsigned char y = 0;
		unsigned char x = 0;
		if (i < half) {
			y = CPU_L1;
			x = (lp1 >> (i << 2)) & 0x0f;
		} else {
			y = CPU_L2;
			x = (lp2 >> ((i - half) << 2)) & 0x0f;
		}
		::SetPixel(hdcBuff, x, y, color);
		::SetPixel(hdcBuff, x + 1, y, color);
	}

	ICONINFO ii = {0};
	ii.fIcon = TRUE;
	ii.hbmMask = this->hbmMask;
	ii.hbmColor = hbmBuff;

	HICON hIcon = ::CreateIconIndirect(&ii);
	::SelectObject(hdcChar, hbmOldChar);
	::SelectObject(hdcBuff, hbmOldBuff);

	::DeleteObject(hbmChar);
	::DeleteObject(hbmBuff);

	::DeleteDC(hdcChar);
	::DeleteDC(hdcBuff);
	::ReleaseDC(NULL, hdc);

	return hIcon;
}

void PerformanceIcon::makeCpuChars(char percentage, char *buff)
{
	memset(buff, ' ', CHAR_N);
	memcpy(buff + 2, "0%", 2);
	for (int i = CHAR_N - 2; i >= 0 && percentage > 0; i--) {
		buff[i] = percentage % 10 + '0';
		percentage /= 10;
	}
}

void PerformanceIcon::makeMemoryChars(int dispMemory, char unit, char *buff)
{
	if ((dispMemory >> 10) < 10) {
		sprintf_s(buff, CHAR_N, "%1d.%1d", dispMemory >> 10, ((dispMemory & 0x3ff) * 10) >> 10);
	} else {
		sprintf_s(buff, CHAR_N, "%3d", dispMemory >> 10);
	}
	buff[CHAR_N - 1] = unit;
}

void HSV2RGB(int h, int s, int v, COLORREF* rgb)
{
	if (s == 0)
	{
		*rgb = RGB(v, v, v);
		return;
	}

	int hi = h % 60;
	int f = ((hi << 8) - hi) / 60; 
	h /= 60;

	long p = (v * (256 - s)) >> 8;
	long q = (v * (256 - ((s * f) >> 8))) >> 8;
	long t = (v * (256 - ((s * (256 - f)) >> 8))) >> 8;

	switch(h) {
		case 0:
			*rgb = RGB(v, t, p);
			break;
		case 1:
			*rgb = RGB(q, v, p);
			break;
		case 2:
			*rgb = RGB(p, v, t);
			break;
		case 3:
			*rgb = RGB(p, q, v);
			break;
		case 4:
			*rgb = RGB(t, p, v);
			break;
		default:
			*rgb = RGB(v, p, q);
	}
}