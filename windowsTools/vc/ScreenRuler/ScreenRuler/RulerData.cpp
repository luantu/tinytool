#include "StdAfx.h"
#include "RulerData.h"

#define DEFAULT_DISTANCE_FORMAT	_T("%.2f(%4d,%4d)")
#define DEFAULT_POINT_FORMAT	_T("[%4d,%4d]")
#define FORMAT_LENGTH			256

RulerData::RulerData(void)
{
	this->szDistanceFormat = new TCHAR[FORMAT_LENGTH]; 
	this->szPointFormat = new TCHAR[FORMAT_LENGTH]; 
	_tcscpy_s(this->szDistanceFormat, FORMAT_LENGTH, DEFAULT_DISTANCE_FORMAT); 
	_tcscpy_s(this->szPointFormat, FORMAT_LENGTH, DEFAULT_POINT_FORMAT); 
}

RulerData::~RulerData(void)
{
}

double RulerData::GetDistance()
{
	double dx = this->endPt.x - this->startPt.x; 
	double dy = this->endPt.y - this->startPt.y; 
	return sqrt(dx * dx + dy * dy); 
}

void RulerData::GetDistance(TCHAR **pszDistance, size_t len)
{
	int dx = abs(this->endPt.x - this->startPt.x); 
	int dy = abs(this->endPt.y - this->startPt.y); 
	_stprintf_s(*pszDistance, len, this->szDistanceFormat, sqrt(0. + dx * dx + dy * dy), dx, dy); 
}

void RulerData::GetDistance(double distance, TCHAR **pszDistance, size_t len)
{
	int dx = abs(this->endPt.x - this->startPt.x); 
	int dy = abs(this->endPt.y - this->startPt.y); 
	_stprintf_s(*pszDistance, len, this->szDistanceFormat, distance, dx, dy); 
}

void RulerData::GetStartPoint(TCHAR **pszStartPoint, size_t len)
{
	_stprintf_s(*pszStartPoint, len, this->szPointFormat, this->startPt.x, this->startPt.y); 
}

void RulerData::GetEndPoint(TCHAR **pszEndPoint, size_t len)
{
	_stprintf_s(*pszEndPoint, len, this->szPointFormat, this->endPt.x, this->endPt.y); 
}
