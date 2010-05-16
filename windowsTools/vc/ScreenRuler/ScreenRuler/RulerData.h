#pragma once

#define PT_NUM 2

class RulerData
{
public:
	RulerData(void);
	~RulerData(void);

public:
	POINT startPt; 
	POINT endPt; 
	TCHAR* szPointFormat; 
	TCHAR* szDistanceFormat;

public:
	double GetDistance(); 
	void GetDistance(__out TCHAR** pszDistance, __in size_t len); 
	void GetDistance(__in double distance, __out TCHAR** pszDistance, __in size_t len); 
	void GetStartPoint(__out TCHAR** pszStartPoint, __in size_t len); 
	void GetEndPoint(__out TCHAR** pszEndPoint, __in size_t len); 
};
