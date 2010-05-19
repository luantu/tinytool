#include "StdAfx.h"
#include "VisualRulerData.h"

VisualRulerData::VisualRulerData(__in HWND hWnd)
{
	this->m_hWnd = hWnd; 
	HINSTANCE hInst = ::GetModuleHandle(NULL); 

	this->m_focusPointFlag = 3; 
	this->m_showCross = FALSE; 
	this->m_bLocked = FALSE; 

	this->m_pointColor = RGB(0, 0, 0); 
	this->m_focusPointColor = RGB(255, 0, 0); 
	this->m_disLineColor = RGB(100, 100, 255); 
	this->m_lockedLineColor = RGB(192, 192, 192); 
	this->m_textColor = RGB(254, 254, 254); 
	this->m_disTextColor = RGB(255, 255, 128); 
	this->m_crossLineColor = RGB(255, 128, 0); 

	this->m_hbmPoint = ::LoadBitmap(hInst, MAKEINTRESOURCE(IDB_POINT)); 
	this->m_hbmFocusPoint = ::LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FOCUS_POINT)); 
	this->m_hbmLabel = ::LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LABEL)); 
	this->m_bLabelBMReady = FALSE; 

	this->m_pointSize.cx = 21;
	this->m_pointSize.cy = 21; 
	this->m_labelSize.cx = 100; 
	this->m_labelSize.cy = 32;
	this->m_labelOffset.cx = 8; 
	this->m_labelOffset.cy = 11; 
	this->m_orientation = 0; 
	this->m_lineLableDist = 100;

	RECT windowRect; 
	::GetWindowRect(hWnd, &windowRect); 
	this->m_boundary.cx = windowRect.right - windowRect.left;
	this->m_boundary.cy = windowRect.bottom - windowRect.top; 

	this->CalculateCapture();
}

VisualRulerData::~VisualRulerData(void)
{
}

int VisualRulerData::GetStartOrientation()
{
	return this->m_orientation & 0x03; 
}

int VisualRulerData::GetEndOrientation()
{
	return this->m_orientation >> 2; 
}

int VisualRulerData::GetLineLabelOrientation()
{
	return this->m_orientation >> 4; 
}

void VisualRulerData::SetStartOrientation(__in int orientation)
{
	this->m_orientation = this->m_orientation & ~0x03 | orientation; 
}

void VisualRulerData::SetEndOrientation(__in int orientation)
{
	this->m_orientation = this->m_orientation & ~0x0c | (orientation << 2); 
}

void VisualRulerData::SetLineLabelOrientation(__in int orientation)
{
	this->m_orientation = this->m_orientation & ~0x30 | (orientation << 4); 
}

void VisualRulerData::CalculateCapture()
{
	this->m_captureSize.cx = m_pointSize.cx + m_labelOffset.cx + m_labelSize.cx; 
	this->m_captureSize.cy = (m_pointSize.cy + (m_labelOffset.cy << 1) + m_labelSize.cy) >> 1; 

	m_captureOffsets[LO_TOPRIGHT].cx	= m_captureOffsets[LO_BOTTOMRIGHT].cx
		= -(m_pointSize.cx >> 1); 
	m_captureOffsets[LO_TOPRIGHT].cy	= m_captureOffsets[LO_TOPLEFT].cy
		= -(m_labelOffset.cy + (m_labelSize.cy >> 1)); 
	m_captureOffsets[LO_TOPLEFT].cx		= m_captureOffsets[LO_BOTTOMLEFT].cx
		= -((m_pointSize.cx >> 1) + m_labelOffset.cx + m_labelSize.cx); 
	m_captureOffsets[LO_BOTTOMLEFT].cy	= m_captureOffsets[LO_BOTTOMRIGHT].cy
		= -(m_pointSize.cy >> 1); 
}

BOOL VisualRulerData::DrawPoint(__in HDC hdc, __in HDC hdcMem, __in POINT pt, __in BOOL bFocus)
{
	// if cross needed to be shown, 
	if (this->m_showCross)
	{	// draw it
		// draw horizontal line
		COLORREF oldPenColor = ::SetDCPenColor(hdc, this->m_bLocked ? this->m_lockedLineColor: this->m_crossLineColor); 
		HGDIOBJ hOldPen = ::SelectObject(hdc, ::GetStockObject(DC_PEN)); 
		int dx = this->m_pointSize.cx >> 1;
		int dy = this->m_pointSize.cy >> 1; 
		::MoveToEx(hdc, 0, pt.y, NULL); 
		::LineTo(hdc, pt.x - dx, pt.y); 
		::MoveToEx(hdc, pt.x + dx, pt.y, NULL); 
		::LineTo(hdc, this->m_boundary.cx, pt.y); 
		::MoveToEx(hdc, pt.x, 0, NULL); 
		::LineTo(hdc, pt.x, pt.y - dy); 
		::MoveToEx(hdc, pt.x, pt.y + dy, NULL); 
		::LineTo(hdc, pt.x, this->m_boundary.cy); 
		::SelectObject(hdc, hOldPen); 
		::SetDCPenColor(hdc, oldPenColor); 
	}
	// Load bitmap
	BITMAP bm; 
	HBITMAP hbm = bFocus ? this->m_hbmFocusPoint : this->m_hbmPoint; 

	HBITMAP hbmOld = (HBITMAP) ::SelectObject(hdcMem, hbm); 
	::GetObject(hbm, sizeof(bm), &bm); 

	// recalculate the size
	BOOL bSizeChanged = ((this->m_pointSize.cx == bm.bmWidth) || (this->m_pointSize.cy == bm.bmHeight)); 
	if (bSizeChanged)
	{
		this->m_pointSize.cx = bm.bmWidth; 
		this->m_pointSize.cy = bm.bmHeight; 
	}

	// draw bitmap
	int dx = m_pointSize.cx >> 1;
	int dy = m_pointSize.cy >> 1; 
	BitBlt(hdc, pt.x - dx, pt.y - dy, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCAND); 

	::SelectObject(hdcMem, hbmOld); 

	// draw the point
	COLORREF oldPenColor = ::SetDCPenColor(hdc, bFocus ? this->m_focusPointColor : this->m_pointColor); 
	HGDIOBJ hOldPen = ::SelectObject(hdc, ::GetStockObject(DC_PEN)); 
	MoveToEx(hdc, pt.x, pt.y, NULL); 
	LineTo(hdc, pt.x, pt.y + 1); 
	::SelectObject(hdc, hOldPen); 
	::SetDCPenColor(hdc, oldPenColor); 


	return bSizeChanged; 
}

POINT VisualRulerData::GetPointLabelPos(__in POINT pt, __in int orientation)
{
	POINT labelPt; 
	int ppcx = (this->m_pointSize.cx + (this->m_labelOffset.cx << 1) + this->m_labelSize.cx) >> 1; 
	int ppcy = this->m_labelOffset.cy; 
	labelPt.x = pt.x + (orientation & LOM_RIGHTLEFT ? -ppcx : ppcx); 
	labelPt.y = pt.y + (orientation & LOM_TOPBOTTOM ? ppcy : -ppcy); 
	return labelPt; 
}

RECT VisualRulerData::GetPointLabelRect(__in POINT pt, __in int orientation)
{
	POINT labelPt; 
	int ppcx = (this->m_pointSize.cx + (this->m_labelOffset.cx << 1) + this->m_labelSize.cx) >> 1; 
	int ppcy = this->m_labelOffset.cy; 
	labelPt.x = pt.x + (orientation & LOM_RIGHTLEFT ? -ppcx : ppcx); 
	labelPt.y = pt.y + (orientation & LOM_TOPBOTTOM ? ppcy : -ppcy); 
	int w = m_labelSize.cx >> 1;
	int h = m_labelSize.cy >> 1;
	RECT rect = {pt.x - w, pt.y - h, pt.x + w, pt.y + h}; 
	return rect; 
}

BOOL VisualRulerData::PrepareLabelMemDC(__inout HDC hdcMem)
{
	BOOL bSizeChanged = FALSE; 
	if (this->m_bLabelBMReady)
	{
		::SelectObject(hdcMem, this->m_hbmLabel); 
	}
	else
	{
		// load bitmap
		BITMAP bm; 

		::SelectObject(hdcMem, this->m_hbmLabel); 
		::GetObject(this->m_hbmLabel, sizeof(bm), &bm); 

		// recalculate the size
		bSizeChanged = ((this->m_labelSize.cx != bm.bmWidth) || (this->m_labelSize.cy != bm.bmHeight)); 
		if (bSizeChanged)
		{
			this->m_labelSize.cx = bm.bmWidth; 
			this->m_labelSize.cy = bm.bmHeight; 
		}
		BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, bm.bmWidth, bm.bmHeight, PATINVERT); 
		this->m_bLabelBMReady = TRUE; 
	}

	return bSizeChanged; 
}

void VisualRulerData::DrawLabel(__in HDC hdc, __in HDC hdcMem, __in POINT pt, __in TCHAR* szText)
{
	this->DrawLabel(hdc, hdcMem, pt, szText, this->m_textColor); 
}

void VisualRulerData::DrawLabel(__in HDC hdc, __in HDC hdcMem, __in POINT pt, __in TCHAR* szText, COLORREF color)
{
	// calculate draw point
	int px = pt.x; 
	int py = pt.y; 
	int dx = this->m_labelSize.cx >> 1; 
	int dy = this->m_labelSize.cy >> 1; 

	// draw bitmap
	BitBlt(hdc, px - dx, py - dy, this->m_labelSize.cx, this->m_labelSize.cy, hdcMem, 0, 0, SRCINVERT); 

	// write text
	HFONT hFont = (HFONT) ::GetStockObject(ANSI_FIXED_FONT); 
	HFONT hFontOld = (HFONT) ::SelectObject(hdc, hFont); 
	COLORREF oldColor = ::SetTextColor(hdc, color); 
	::SetBkMode(hdc, TRANSPARENT); 
	RECT rect = {px - dx, py - dy, px + dx, py + dy}; 
	::DrawText(hdc, szText, (int) _tcslen(szText), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE); 
	::SelectObject(hdc, hFontOld); 
	::SetTextColor(hdc, oldColor); 
}

void VisualRulerData::DrawDistanceLine(__in HDC hdc, __in HDC hdcMem)
{
	// draw the line
	COLORREF oldPenColor = ::SetDCPenColor(hdc, this->m_bLocked ? this->m_lockedLineColor : this->m_disLineColor); 
	HGDIOBJ hOldPen = ::SelectObject(hdc, ::GetStockObject(DC_PEN)); 
	MoveToEx(hdc, this->startPt.x, this->startPt.y, NULL); 
	LineTo(hdc, this->endPt.x, this->endPt.y); 
	::SelectObject(hdc, hOldPen); 
	::SetDCPenColor(hdc, oldPenColor); 

	// draw the label
	POINT midPt; 
	midPt.x = (this->startPt.x + this->endPt.x) >> 1; 
	midPt.y = (this->startPt.y + this->endPt.y) >> 1; 

	TCHAR* szText = NULL; 
	szText = new TCHAR[64]; 
	this->GetDistance(&szText, 64); 

	POINT pt = {midPt.x + m_lineLabelOffset.cx, midPt.y + m_lineLabelOffset.cy}; 
	this->DrawLabel(hdc, hdcMem, pt, szText, this->m_disTextColor); 

	delete[] szText; 
}

void VisualRulerData::Draw()
{
	PAINTSTRUCT ps; 
	HDC hdc = ::BeginPaint(this->m_hWnd, &ps); 
	// ----------------------------------------------
	//  Draw the ruler
	// ----------------------------------------------

	HDC hdcMem4Point = ::CreateCompatibleDC(hdc); 
	HDC hdcMem4Label = ::CreateCompatibleDC(hdc); 

	TCHAR* szText = NULL; 
	szText = new TCHAR[128]; 

	BOOL isSizeChanged = this->PrepareLabelMemDC(hdcMem4Label); 

	this->DrawDistanceLine(hdc, hdcMem4Label); 

	BOOL bStartFocus = this->m_focusPointFlag & FPF_START; 
	BOOL bEndFocus = this->m_focusPointFlag & FPF_END; 

	isSizeChanged = this->DrawPoint(hdc, hdcMem4Point, this->startPt, bStartFocus && !bEndFocus) || isSizeChanged; 
	this->GetStartPoint(&szText, 128); 
	this->DrawLabel(hdc, hdcMem4Label, this->GetPointLabelPos(this->startPt, this->GetStartOrientation()), szText); 

	isSizeChanged = this->DrawPoint(hdc, hdcMem4Point, this->endPt, bEndFocus && !bStartFocus) || isSizeChanged; 
	this->GetEndPoint(&szText, 128); 
	this->DrawLabel(hdc, hdcMem4Label, this->GetPointLabelPos(this->endPt, this->GetEndOrientation()), szText); 

	if (isSizeChanged)
	{
		this->CalculateCapture(); 
	}

	delete[] szText; 
	::DeleteDC(hdcMem4Label); 
	::DeleteDC(hdcMem4Point); 
	// ----------------------------------------------
	::EndPaint(this->m_hWnd, &ps); 
}

void VisualRulerData::AdjustLabelOrientation()
{
	int startOrientation = 0; 
	int endOrientation = 0; 

	int dx = this->endPt.x - this->startPt.x; 
	int dy = this->endPt.y - this->startPt.y; 

	if (dx > 0)
	{
		startOrientation |= 0x02; 
	}
	else
	{
		endOrientation |= 0x02; 
	}

	if (dy > 0)
	{
		endOrientation |= 0x01; 
	}
	else
	{
		startOrientation |= 0x01;
	}

	this->AdjustSinglePointLabel(this->startPt, &startOrientation); 
	this->AdjustSinglePointLabel(this->endPt, &endOrientation); 

	this->SetStartOrientation(startOrientation); 
	this->SetEndOrientation(endOrientation); 
}

void VisualRulerData::AdjustSinglePointLabel(__in POINT pt, __inout int* po)
{
	RECT rect = {
		pt.x + this->m_captureOffsets[*po].cx, 
		pt.y + this->m_captureOffsets[*po].cy, 
		pt.x + this->m_captureOffsets[*po].cx + this->m_captureSize.cx, 
		pt.y + this->m_captureOffsets[*po].cy + this->m_captureSize.cy,
	}; 
	if (rect.top < 0)
	{
		*po |= 0x01; 
	}
	if (rect.bottom > m_boundary.cy)
	{
		*po &= ~0x01; 
	}
	if (rect.left < 0)
	{
		*po &= ~0x02; 
	}
	if (rect.right > m_boundary.cx)
	{
		*po |= 0x02; 
	}
}

void VisualRulerData::AdjustLineLabelOrientation()
{
	int dx = endPt.x - startPt.x; 
	int dy = endPt.y - startPt.y; 
	double distance = this->GetDistance(); 
	if (distance == 0)
	{
		distance = 0.001;
	}
	int ndx = (int) (this->m_lineLableDist * dy / distance); 
	int ndy = (int) (-this->m_lineLableDist * dx / distance); 
	ndy = (ndy >> 2) + (ndy >> 3); 
	int w = m_labelSize.cx >> 1; 
	int h = m_labelSize.cy >> 1; 
	POINT midPt; 
	midPt.x = (this->startPt.x + this->endPt.x) >> 1; 
	midPt.y = (this->startPt.y + this->endPt.y) >> 1; 

	RECT startLabelRect = this->GetPointLabelRect(this->startPt, this->GetStartOrientation()); 
	RECT endLabelRect = this->GetPointLabelRect(this->endPt, this->GetEndOrientation()); 

	POINT pt = {midPt.x + ndx, midPt.y + ndy}; 
	LONG left = pt.x - w; 
	LONG top = pt.y - h; 
	LONG right = pt.x + w; 
	LONG bottom = pt.y + h; 
	if (left < 0)
	{
		ndx = abs(ndx); 
	}
	if (top < 0)
	{
		ndy = abs(ndy); 
	}
	if (right > m_boundary.cx)
	{
		ndx = -abs(ndx); 
	}
	if (bottom > m_boundary.cy)
	{
		ndy = -abs(ndy); 
	}
	this->m_lineLabelOffset.cx = ndx; 
	this->m_lineLabelOffset.cy = ndy; 
}

void VisualRulerData::GetPointCaptureRect(__in POINT pt, __in int orientation, __out RECT* pRect)
{
	RECT rect = {
		pt.x + this->m_captureOffsets[orientation].cx, 
		pt.y + this->m_captureOffsets[orientation].cy, 
		pt.x + this->m_captureOffsets[orientation].cx + this->m_captureSize.cx, 
		pt.y + this->m_captureOffsets[orientation].cy + this->m_captureSize.cy, 
	}; 
	*pRect = rect; 
}

void VisualRulerData::GetStartPointCaptureRect(__out RECT* pRect)
{
	this->GetPointCaptureRect(this->startPt, this->GetStartOrientation(), pRect); 
}

void VisualRulerData::GetEndPointCaptureRect(__out RECT* pRect)
{
	this->GetPointCaptureRect(this->endPt, this->GetEndOrientation(), pRect); 
}

BOOL VisualRulerData::IsPointIntoRect(__in POINT* pt, __in RECT* pRect)
{
	return this->IsPointIntoRect(&(pt->x), &(pt->y), pRect); 
}

BOOL VisualRulerData::IsPointIntoRect(__in LONG* x, __in LONG* y, __in RECT* pRect)
{
	return (*x >= pRect->left) && (*x <= pRect->right) && (*y >= pRect->top) && (*y <= pRect->bottom); 
}

BOOL VisualRulerData::IsPointIntoStartPoint(__in POINT* pt)
{
	RECT rect; 
	this->GetStartPointCaptureRect(&rect); 
	return this->IsPointIntoRect(pt, &rect); 
}

BOOL VisualRulerData::IsPointIntoStartPoint(__in LONG* x, __in LONG* y)
{
	RECT rect; 
	this->GetStartPointCaptureRect(&rect); 
	return this->IsPointIntoRect(x, y, &rect); 
}

BOOL VisualRulerData::IsPointIntoEndPoint(__in POINT* pt)
{
	RECT rect; 
	this->GetEndPointCaptureRect(&rect); 
	return this->IsPointIntoRect(pt, &rect); 
}

BOOL VisualRulerData::IsPointIntoEndPoint(__in LONG* x, __in LONG* y)
{
	RECT rect; 
	this->GetStartPointCaptureRect(&rect); 
	return this->IsPointIntoRect(x, y, &rect); 
}

