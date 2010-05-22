#pragma once
#include "stdafx.h"
#include "rulerdata.h"
#include "resource.h"

#define LO_TOPRIGHT		0
#define LO_BOTTOMRIGHT	1
#define LO_TOPLEFT		2
#define LO_BOTTOMLEFT	3

#define LOM_TOPBOTTOM	1
#define LOM_RIGHTLEFT	2

#define FPF_NONE		0
#define	FPF_START		1
#define FPF_END			2
#define FPF_BOTH		3


class VisualRulerData :
	public RulerData
{
public:
	VisualRulerData(__in HWND hWnd);
	~VisualRulerData(void);

protected:
	HWND		m_hWnd; 
	SIZE		m_pointSize;
	SIZE		m_labelSize;
	SIZE		m_labelOffset;
	int			m_lineLableDist; 
	SIZE		m_captureSize;
	SIZE		m_captureOffsets[4]; 

	SIZE		m_lineLabelOffset; 

	int			m_orientation; 

	HBITMAP		m_hbmPoint; 
	HBITMAP		m_hbmFocusPoint;
	HBITMAP		m_hbmLabel;
	BOOL		m_bLabelBMReady; 

public:
	COLORREF	m_disLineColor; 
	COLORREF	m_lockedLineColor; 
	COLORREF	m_pointColor; 
	COLORREF	m_focusPointColor;
	COLORREF	m_textColor; 
	COLORREF	m_disTextColor; 
	COLORREF	m_crossLineColor;
	COLORREF	m_pointInMagnifierColor;
	COLORREF	m_pointInMagnifierAltColor; 
	COLORREF	m_magnifierBorderColor; 

	SIZE		m_boundary; 
	int			m_focusPointFlag; 
	BOOL		m_showCross; 
	BOOL		m_bLocked; 
	BOOL		m_bMagnifier; 
	BOOL		m_bMoving;

protected:
	BOOL PrepareLabelMemDC(__inout HDC hdcMem); 
	BOOL DrawPoint(__in HDC hdc, __in HDC hdcMem, __in POINT pt, __in BOOL bFocus); 
	POINT GetPointLabelPos(__in POINT pt, __in int orientation); 
	RECT GetPointLabelRect(__in POINT pt, __in int orientation); 

	void DrawLabel(__in HDC hdc, __in HDC hdcMem, __in POINT pt, __in TCHAR* szText);
	void DrawLabel(__in HDC hdc, __in HDC hdcMem, __in POINT pt, __in TCHAR* szText, __in COLORREF color);
	void DrawDistanceLine(__in HDC hdc, __in HDC hdcMem);

	POINT GetMagnifierPos(__in POINT* pLabelPt, __in int orientation); 
	void DrawMagnifier(__in HDC hdc, __in HDC hdcMem, __in POINT* pLabelPt, __in POINT* ppt); 

	void CalculateCapture(); 

	int GetStartOrientation(); 
	int GetEndOrientation(); 
	int GetLineLabelOrientation(); 
	void SetStartOrientation(__in int orientation); 
	void SetEndOrientation(__in int orientation); 
	void SetLineLabelOrientation(__in int orientation); 

	void AdjustSinglePointLabel(__in POINT pt, __inout int* po); 

	void GetPointCaptureRect(__in POINT pt, __in int orientation, __out RECT* pRect); 
	BOOL IsPointIntoRect(__in POINT* pt, __in RECT* pRect); 
	BOOL IsPointIntoRect(__in LONG* x, __in LONG* y, __in RECT* pRect); 

public:
	void AdjustLabelOrientation(); 
	void AdjustLineLabelOrientation(); 
	void Draw(); 

	void GetStartPointCaptureRect(__out RECT* pRect); 
	BOOL IsPointIntoStartPoint(__in POINT* pt); 
	BOOL IsPointIntoStartPoint(__in LONG* x, __in LONG* y); 
	void GetEndPointCaptureRect(__out RECT* pRect); 
	BOOL IsPointIntoEndPoint(__in POINT* pt); 
	BOOL IsPointIntoEndPoint(__in LONG* x, __in LONG* y); 
};
