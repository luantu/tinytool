#pragma once
#include "stdafx.h"

#define ICON_W		16
#define ICON_H		16

#define CHAR_N		4
#define CHAR_W		3
#define CHAR_H		5
#define CHAR_GAP	1
#define LINETOP_0	0
#define LINETOP_1	11
#define WEEKTOP		6
#define GAP_LEFT	0

#define WEEKDAY_W	15
#define WEEKDAY_H	4

#define DATE_COLOR	(RGB(0x00, 0x00, 0x00))
#define TIME_COLOR	(RGB(0x00, 0x00, 0x00))

#define WEEKDAY_STR_BEGIN	IDS_SUN
#define WEEKDAY_STR_SIZE	8

#define TIP_LEN		(10 + 4 + 5 + ##WEEKDAY_STR_SIZE)
#define DATE_LEN	4
#define TIME_LEN	4

enum SEC_TYPE
{
	NORMAL,
	LINE1423,
	L14L23,
	STYPE,
	ROLL,
	RANDOM,

	SEC_TYPE_COUNT,
};

class IconFactory
{
private:
	HBRUSH		dateBrush;
	HBRUSH		timeBrush;
	HBITMAP		hbmAscii;
	HBITMAP		hbmWeekdays;
	SYSTEMTIME	sysTime;
	SYSTEMTIME	prevSysTime;

	static const RECT rcChar;
	static const RECT rcIcon;
	unsigned char secType;

protected:
	void set2dtoa(WORD i, char* mem);
	void generatePArray(unsigned char* arr, size_t size);

public:
	IconFactory(COLORREF dateColor, COLORREF timeColor);
	~IconFactory(void);

	BOOL update();
	HICON getIcon();
	int cpyTip(TCHAR* szTip, size_t size);
	void switchSecType();
};
