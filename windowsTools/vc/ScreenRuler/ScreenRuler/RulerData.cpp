#include "StdAfx.h"
#include "RulerData.h"

#define DEFAULT_DISTANCE_FORMAT	_T("%.2f(%4d,%4d)")
#define DEFAULT_POINT_FORMAT	_T("[%4d,%4d]")
#define FORMAT_LENGTH			256

RulerData::RulerData(void)
{
	this->startPt.x = this->startPt.y = this->endPt.x = this->endPt.y = 0; 
	this->prevStartPt = this->startPt; 
	this->prevEndPt = this->endPt; 
	this->prevDistance = 0.; 

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
	if (this->startPt.x != this->prevStartPt.x || this->endPt.x != this->prevEndPt.x || this->startPt.y != this->prevStartPt.y || this->endPt.y != this->prevEndPt.y)
	{
		this->prevDx = this->endPt.x - this->startPt.x; 
		this->prevDy = this->endPt.y - this->startPt.y; 
		this->prevDistance = sqrt(0. + this->prevDx * this->prevDx + this->prevDy * this->prevDy); 

		this->prevStartPt = this->startPt; 
		this->prevEndPt = this->endPt; 
	}
	return this->prevDistance; 
}

void RulerData::GetDistance(TCHAR **pszDistance, size_t len)
{
	_stprintf_s(*pszDistance, len, this->szDistanceFormat, this->GetDistance(), this->prevDx, this->prevDy); 
}

void RulerData::GetStartPoint(TCHAR **pszStartPoint, size_t len)
{
	_stprintf_s(*pszStartPoint, len, this->szPointFormat, this->startPt.x, this->startPt.y); 
}

void RulerData::GetEndPoint(TCHAR **pszEndPoint, size_t len)
{
	_stprintf_s(*pszEndPoint, len, this->szPointFormat, this->endPt.x, this->endPt.y); 
}
