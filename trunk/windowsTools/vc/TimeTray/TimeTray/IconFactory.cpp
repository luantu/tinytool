#include "IconFactory.h"
#include "resource.h"
#include <stdio.h>

const RECT IconFactory::rcChar = {0, 0, CHAR_W, CHAR_H};
const RECT IconFactory::rcIcon = {0, 0, ICON_W, ICON_H};

IconFactory::IconFactory(COLORREF dateColor, COLORREF timeColor)
{
	this->hbmAscii = ::LoadBitmap(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ASCII)); 
	this->hbmWeekdays = ::LoadBitmap(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_WEEKDAYS));
	this->dateBrush = ::CreateSolidBrush(dateColor);
	this->timeBrush = ::CreateSolidBrush(timeColor);
	memset(&this->sysTime, 0, sizeof(this->sysTime));
	memset(&this->prevSysTime, 0, sizeof(this->prevSysTime));
	this->secType = RANDOM;
}

IconFactory::~IconFactory(void)
{
	::DeleteObject(this->hbmAscii);
	::DeleteObject(this->hbmWeekdays);
	::DeleteObject(this->dateBrush);
	::DeleteObject(this->timeBrush);
}

BOOL IconFactory::update()
{
	::GetLocalTime(&this->sysTime);
	BOOL bSame = memcmp(&this->sysTime, &this->prevSysTime, sizeof(SYSTEMTIME)) == 0;
	if (!bSame) {
		memcpy(&this->prevSysTime, &this->sysTime, sizeof(SYSTEMTIME));
	}
	return !bSame;
}

void IconFactory::set2dtoa(WORD i , char* mem)
{
	if (i >= 10) {
		int ii = i / 10;
		mem[0] = '0' + ii;
		mem[1] = '0' + i - ((ii << 3) + (ii << 1)); // (i % 10)
	} else {
		mem[0] = '0';
		mem[1] = '0' + i;
	}
}

HICON IconFactory::getIcon()
{
	HDC hdc = ::GetDC(NULL);

	HDC hdcMask = ::CreateCompatibleDC(hdc);
	HBITMAP hbmMask = ::CreateCompatibleBitmap(hdc, ICON_W, ICON_H);
	HBITMAP oldMask = SelectBitmap(hdcMask, hbmMask);

	HDC hdcColor = ::CreateCompatibleDC(hdc);
	HBITMAP hbmColor = ::CreateCompatibleBitmap(hdc, ICON_W, ICON_H);
	HBITMAP oldColor = SelectBitmap(hdcColor, hbmColor);

	HDC hdcChar = ::CreateCompatibleDC(hdc);
	HBITMAP hbmChar = ::CreateCompatibleBitmap(hdc, CHAR_W, CHAR_H);
	HBITMAP oldChar = SelectBitmap(hdcChar, hbmChar);

	HDC hdcWeekdays = ::CreateCompatibleDC(hdc);
	HBITMAP oldWeekdays = SelectBitmap(hdcWeekdays, this->hbmWeekdays);

	HDC hdcAscii = ::CreateCompatibleDC(hdc);
	HBITMAP oldAscii = SelectBitmap(hdcAscii, this->hbmAscii);

	// init mask as transparent
	::FillRect(hdcMask, &rcIcon, GetStockBrush(WHITE_BRUSH));
	::FillRect(hdcColor, &rcIcon, GetStockBrush(BLACK_BRUSH));


	// generate date buffer
	char buff[CHAR_N] = {0};
	this->set2dtoa(this->sysTime.wMonth, buff);
	this->set2dtoa(this->sysTime.wDay, buff + 2);
	::FillRect(hdcChar, &rcChar, this->dateBrush);
	for (char i = 0; i < CHAR_N; i++) {
		int x = GAP_LEFT + (CHAR_W + CHAR_GAP) * i;
		int y = LINETOP_0;
		int yMask = CHAR_H * (buff[i] - ' ');
		::BitBlt(hdcMask, x, y, CHAR_W, CHAR_H, hdcAscii, 0, yMask, SRCCOPY);
		::MaskBlt(hdcColor, x, y, CHAR_W, CHAR_H, hdcChar, 0, 0, this->hbmAscii, 0, yMask, MAKEROP4(BLACKNESS, SRCCOPY));
	}

	// generate time buffer
	this->set2dtoa(this->sysTime.wHour, buff);
	this->set2dtoa(this->sysTime.wMinute, buff + 2);
	::FillRect(hdcChar, &rcChar, this->timeBrush);
	for (char i = 0; i < CHAR_N; i++) {
		int x = GAP_LEFT + (CHAR_W + CHAR_GAP) * i;
		int y = LINETOP_1;
		int yMask = CHAR_H * (buff[i] - ' ');
		::BitBlt(hdcMask, x, y, CHAR_W, CHAR_H, hdcAscii, 0, yMask, SRCCOPY);
		::MaskBlt(hdcColor, x, y, CHAR_W, CHAR_H, hdcChar, 0, 0, this->hbmAscii, 0, yMask, MAKEROP4(BLACKNESS, SRCCOPY));
	}

	// draw weekdays
	int yOffset = this->sysTime.wDayOfWeek * WEEKDAY_H;
	::BitBlt(hdcColor, GAP_LEFT, WEEKTOP, WEEKDAY_W, WEEKDAY_H, hdcWeekdays, 0, yOffset, SRCCOPY);
	::BitBlt(hdcMask, GAP_LEFT, WEEKTOP, WEEKDAY_W, WEEKDAY_H, hdcWeekdays, 0, yOffset, BLACKNESS);

	// present seconds
	static unsigned char secPArray[WEEKDAY_W * WEEKDAY_H] = {0};
	if (this->secType & 0x80 || secPArray[0] == secPArray[1] || this->sysTime.wSecond == 0) {
		this->generatePArray(secPArray, sizeof(secPArray));
	}

	for (char i = 0; i < this->sysTime.wSecond; i++) {
		int x = GAP_LEFT + (secPArray[i] & 0x0f);
		int y = (secPArray[i] >> 4) + WEEKTOP;
		::SetPixel(hdcMask, x, y, RGB(0xff, 0xff, 0xff));
		::SetPixel(hdcColor, x, y, RGB(0, 0, 0));
	}


	SelectBitmap(hdcChar, oldChar);
	SelectBitmap(hdcColor, oldColor);
	SelectBitmap(hdcMask, oldMask);
	SelectBitmap(hdcWeekdays, oldWeekdays);
	SelectBitmap(hdcAscii, oldAscii);

	// generate icon
	ICONINFO ii = {0};
	ii.fIcon = TRUE;
	ii.hbmMask = hbmMask;
	ii.hbmColor = hbmColor;
	
	HICON hIcon = ::CreateIconIndirect(&ii);

	DeleteBitmap(hbmColor);
	DeleteBitmap(hbmMask);
	DeleteBitmap(hbmChar);

	::DeleteDC(hdcChar);
	::DeleteDC(hdcColor);
	::DeleteDC(hdcMask);
	::DeleteDC(hdcWeekdays);
	::DeleteDC(hdcAscii);
	::ReleaseDC(NULL, hdc);

	return hIcon;
}

void IconFactory::generatePArray(unsigned char* arr, size_t size)
{
	const static unsigned char normal[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 
	};
	const static unsigned char line1423[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 
	};
	const static unsigned char l14l23[] = {
		0x00, 0x30, 
		0x01, 0x31, 
		0x02, 0x32, 
		0x03, 0x33, 
		0x04, 0x34, 
		0x05, 0x35, 
		0x06, 0x36, 
		0x07, 0x37, 
		0x08, 0x38, 
		0x09, 0x39, 
		0x0a, 0x3a, 
		0x0b, 0x3b, 
		0x0c, 0x3c, 
		0x0d, 0x3d, 
		0x0e, 0x3e, 
		0x10, 0x20, 
		0x11, 0x21, 
		0x12, 0x22, 
		0x13, 0x23, 
		0x14, 0x24, 
		0x15, 0x25, 
		0x16, 0x26, 
		0x17, 0x27, 
		0x18, 0x28, 
		0x19, 0x29, 
		0x1a, 0x2a, 
		0x1b, 0x2b, 
		0x1c, 0x2c, 
		0x1d, 0x2d, 
		0x1e, 0x2e, 
	};

	const static unsigned char stype[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
		0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
		0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
	};

	const static unsigned char roll[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
		0x1e, 
		0x2e, 
		0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
		0x20, 
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 
		      0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 
	};

	const static unsigned char* secTypesArray[] = {
		normal, 
		line1423, 
		l14l23, 
		stype, 
		roll, 
		normal, // for random
	};

	this->secType &= 0x7f;
	const unsigned char* secTypeArray = secTypesArray[this->secType];

	size_t n = size;
	memcpy(arr, secTypeArray, n);

	if (this->secType == RANDOM) {
		srand(this->sysTime.wYear + this->sysTime.wMonth + this->sysTime.wDay + this->sysTime.wHour + this->sysTime.wMinute + this->sysTime.wDayOfWeek + this->sysTime.wMilliseconds);
		for (size_t i = 0; i < n - 1; i++) {
			size_t r = i + (rand() % (n - i));
			unsigned char t = arr[i];
			arr[i] = arr[r];
			arr[r] = t;
		}
	}
}

int IconFactory::cpyTip(TCHAR *szTip, size_t size)
{
	int offset = 0;
	offset += ::GetDateFormat(LOCALE_USER_DEFAULT, NULL, &this->sysTime, _T("yyyy-MM-dd (ddd)\n"), szTip, (int)size);
	offset--;
	offset += ::GetTimeFormat(LOCALE_USER_DEFAULT, NULL, &this->sysTime, _T("HH:mm:ss"), szTip + offset, (int)(size - offset));
	return offset;
}

void IconFactory::switchSecType()
{
	this->secType++;
	this->secType %= SEC_TYPE_COUNT;
	this->secType |= 0x80;
}