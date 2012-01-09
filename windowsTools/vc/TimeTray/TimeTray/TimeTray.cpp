#include "stdafx.h"
#include "IconFactory.h"
#include "resource.h"

#define TRAYICONID	1//				ID number for the Notify Icon
#define TIMER_ID	1//				Timer ID
#define CLK_TIMER	2//				Click Timer

#define TIMER_DUR	100
#define SWM_TRAYMSG	WM_APP//		the message ID sent to our window

#define SWM_SHOW	WM_APP + 1//	show the window
#define SWM_HIDE	WM_APP + 2//	hide the window
#define SWM_EXIT	WM_APP + 3//	close the window

#define APP_NAME	"Time Tray"
#define MUTEX_SZ	"Time Tray. //"
#define MAX_FPATH	1024
#define MAX_FNAME	256

// Global Variables:
HINSTANCE				hInst;		// current instance
NOTIFYICONDATA			niData;		// notify icon data
BOOL					bMsgBox;
IconFactory*			pIf;	// Factory to get icon

BOOL				InitInstance(HINSTANCE, int);
BOOL				AddSysTray(HWND hWnd);
void				ProcessParameter(LPTSTR lpCmdLine);
INT_PTR CALLBACK	TrayProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;

	// Limit only one instance
	HANDLE hMutex = ::CreateMutex(NULL, FALSE, _T(MUTEX_SZ));
	if (ERROR_ALREADY_EXISTS == ::GetLastError()) {
		if (hMutex) {
			CloseHandle(hMutex);
			hMutex = NULL;
		}
		return FALSE;
	}

	pIf = NULL;

	ProcessParameter(lpCmdLine);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) {
		if (pIf) {
			delete pIf;
			pIf = NULL;
		}
		return FALSE;
	}

	// Minimize physical memory once. 
	::SetProcessWorkingSetSize(::GetCurrentProcess(), -1, -1);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(msg.hwnd,&msg) ) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}

void ProcessParameter(LPTSTR lpCmdLine)
{
	const TCHAR* delimiter = _T(" \t\n\r");
	TCHAR* context = NULL;
	COLORREF dateColor = DATE_COLOR;
	COLORREF timeColor = TIME_COLOR;
	for (TCHAR* p = _tcstok_s(lpCmdLine, delimiter, &context); p; p = _tcstok_s(NULL, delimiter, &context)) {
		if (*p == _T('-')) {
			COLORREF* pColor = NULL;
			if (lstrcmpi(p, _T("-date")) == 0 || lstrcmpi(p, _T("-d")) == 0) {
				pColor = &dateColor;
			}
			if (lstrcmpi(p, _T("-time")) == 0 || lstrcmpi(p, _T("-t")) == 0) {
				pColor = &timeColor;
			}
			if (pColor) {
				TCHAR* sColor = _tcstok_s(NULL, delimiter, &context);
				if (sColor) {
					int n = lstrlen(sColor);
					if (n == 6) {
						__int32 rgb = 0;
						for (unsigned char i = 0; i < n; i++) {
							TCHAR c = sColor[i];
							rgb <<= 4;
							if (c >= _T('0') && c <= _T('9')) {
								rgb += c - _T('0');
							}
							if (c >= _T('a') && c <= _T('f')) {
								rgb += c - _T('a') + 10;
							}
							if (c >= _T('A') && c <= _T('F')) {
								rgb += c - _T('A') + 10;
							}
						}
						*pColor = RGB((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
					}
					if (n == 3) {
						__int32 rgb = 0;
						for (unsigned char i = 0; i < n; i++) {
							TCHAR c = sColor[i];
							unsigned char delta = 0;
							if (c >= _T('0') && c <= _T('9')) {
								delta = c - _T('0');
							}
							if (c >= _T('a') && c <= _T('f')) {
								delta = c - _T('a') + 10;
							}
							if (c >= _T('A') && c <= _T('F')) {
								delta = c - _T('A') + 10;
							}
							rgb <<= 8;
							rgb += (delta << 4) + delta;
						}
						*pColor = RGB((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
					}
				}
			}
		}
	}

	pIf = new IconFactory(dateColor, timeColor);
}

//	Initialize the window and tray icon
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// store instance handle and create dialog
	hInst = hInstance;
	HWND hWnd = CreateWindow(
		_T("STATIC"), 
		NULL, 
		NULL, 
		0, 0, 
		0, 0, 
		HWND_MESSAGE, // Message-only window
		NULL, 
		hInstance, 
		NULL);
	//HWND hWnd = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_DLG_DIALOG),
	//	NULL, (DLGPROC)DlgProc );
	if (!hWnd) return FALSE;
#ifdef _WIN64
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)TrayProc);
#else
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)(LONG_PTR)TrayProc);
#endif

	if (!pIf) {
		pIf = new IconFactory(DATE_COLOR, TIME_COLOR);
	}

	SetTimer(hWnd, TIMER_ID, TIMER_DUR, NULL);

	bMsgBox = FALSE;

	return AddSysTray(hWnd);
}

BOOL AddSysTray(HWND hWnd)
{
	// Fill the NOTIFYICONDATA structure and call Shell_NotifyIcon

	// zero the structure - note:	Some Windows funtions require this but
	//								I can't be bothered which ones do and
	//								which ones don't.
	ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

	niData.cbSize = sizeof(NOTIFYICONDATA);

	// the ID number can be anything you choose
	niData.uID = TRAYICONID;

	// state which structure members are valid
	niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	// load the icon
	//niData.hIcon = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_EYES),
	//	IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),
	//	LR_DEFAULTCOLOR);
	niData.hIcon = pIf && pIf->update() ? pIf->getIcon() : NULL;

	// the window to send messages to and the message to send
	//		note:	the message value should be in the
	//				range of WM_APP through 0xBFFF
	niData.hWnd = hWnd;
    niData.uCallbackMessage = SWM_TRAYMSG;

	// tooltip message
    lstrcpyn(niData.szTip, _T(APP_NAME), sizeof(niData.szTip)/sizeof(TCHAR));

	Shell_NotifyIcon(NIM_ADD,&niData);

	// free icon handle
	if(niData.hIcon && DestroyIcon(niData.hIcon))
		niData.hIcon = NULL;

	// call ShowWindow here to make the dialog initially visible

	return TRUE;
}

INT_PTR CALLBACK TrayProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	static UINT_PTR pClkTimer = NULL;

	switch (message) 
	{
	case SWM_TRAYMSG:
		switch(lParam)
		{
		case WM_LBUTTONDOWN:
			// to prevent the double click message, set a timer and run the process later. 
			pClkTimer = ::SetTimer(hWnd, CLK_TIMER, ::GetDoubleClickTime(), NULL);
			break;
		case WM_LBUTTONDBLCLK:
			{
				// if double click, kill the click timer. 
				::KillTimer(hWnd, pClkTimer);
				pClkTimer = NULL;

				::ShellExecute(NULL, _T("open"), _T("RunDll32.exe"), _T("Shell32.dll,Control_RunDLL timedate.cpl"), NULL, SW_SHOWNORMAL);

				// Comment below code out because often cause a 0xc0000005 error. 
//				::WinExec(("RunDll32.exe Shell32.dll,Control_RunDLL timedate.cpl"), SW_SHOWNORMAL);

				// Minimize physical memory once. 
				::SetProcessWorkingSetSize(::GetCurrentProcess(), -1, -1);
			}
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			{
				if (!bMsgBox) {
					bMsgBox = TRUE;
					if (MessageBox(hWnd, _T("This tool is developed by Programus (programus@gmail.com)\nQuit?"), _T(APP_NAME), MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST) == IDYES) {
						DestroyWindow(hWnd);
					}
					bMsgBox = FALSE;
					// Minimize physical memory once. 
					::SetProcessWorkingSetSize(::GetCurrentProcess(), -1, -1);
				}
			}
			break;
		case WM_RBUTTONDBLCLK:
			break;
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam); 

		switch (wmId)
		{
		case SWM_EXIT:
			DestroyWindow(hWnd);
			break;
		}
		return 1;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		niData.uFlags = 0;
		KillTimer(hWnd, TIMER_ID);
		// free icon handle
		if(niData.hIcon && DestroyIcon(niData.hIcon))
			niData.hIcon = NULL;
		Shell_NotifyIcon(NIM_DELETE,&niData);
		if (pIf) {
			delete pIf;
			pIf = NULL;
		}
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		switch (wParam) {
		case TIMER_ID:
			if (pIf && pIf->update()) {
				niData.hIcon = pIf->getIcon();
				pIf->cpyTip(niData.szTip, sizeof(niData.szTip)/sizeof(TCHAR));
				niData.uFlags = NIF_ICON | NIF_TIP;
				if (!Shell_NotifyIcon(NIM_MODIFY, &niData)) {
					// This means the icon is disappeared because some reason like explorer had been killed.
					// So try to add the icon to taskbar again.
					niData.uFlags |= NIF_MESSAGE;
					Shell_NotifyIcon(NIM_ADD, &niData);
				}
				// free icon handle
				if(niData.hIcon && DestroyIcon(niData.hIcon))
					niData.hIcon = NULL;
			}
			break;
		case CLK_TIMER:
			if (pIf) {
				pIf->switchSecType();
			}
			::KillTimer(hWnd, pClkTimer);
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}
