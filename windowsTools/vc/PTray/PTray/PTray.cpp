#include "stdafx.h"
#include "resource.h"
#include "PerformanceMonitor.h"

#define TRAYICONID	1//				ID number for the Notify Icon
#define TIMER_ID	1//				Timer ID

#define TIMER_DUR	500
#define SWM_TRAYMSG	WM_APP//		the message ID sent to our window
#define MIN_MEM_T	5

#define SWM_SHOW	WM_APP + 1//	show the window
#define SWM_HIDE	WM_APP + 2//	hide the window
#define SWM_EXIT	WM_APP + 3//	close the window

#define MUTEX_SZ	"Performance System Tray. //"


// Global Variables:
HINSTANCE		hInst;	// current instance
NOTIFYICONDATA	niData;	// notify icon data
PerformanceMonitor pm;

BOOL				InitInstance(HINSTANCE, int);
BOOL				AddSysTray(HWND hWnd);
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

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;

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
	SetWindowLongPtr(hWnd, GWL_WNDPROC, (LONG_PTR)TrayProc);

	SetTimer(hWnd, TIMER_ID, TIMER_DUR, NULL);

	pm.next();

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
	//niData.hIcon = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON_P),
	//	IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),
	//	LR_DEFAULTCOLOR);
	niData.hIcon = pm.getIcon();

	// the window to send messages to and the message to send
	//		note:	the message value should be in the
	//				range of WM_APP through 0xBFFF
	niData.hWnd = hWnd;
    niData.uCallbackMessage = SWM_TRAYMSG;

	// tooltip message
    lstrcpyn(niData.szTip, _T("Time flies like an arrow but\n   fruit flies like a banana!"), sizeof(niData.szTip)/sizeof(TCHAR));

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

	switch (message) 
	{
	case SWM_TRAYMSG:
		switch(lParam)
		{
		case WM_LBUTTONDBLCLK:
			MessageBox(hWnd, _T("Tiny tool developed by \n    Programus (programus@gmail.com)"), _T("Performance Tray"), MB_OK | MB_ICONINFORMATION);
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			if (MessageBox(hWnd, _T("Quit?"), _T("Confirmation"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
				DestroyWindow(hWnd);
			}
			break;
		case WM_RBUTTONDBLCLK:
			break;
		}
		break;
	//case WM_SYSCOMMAND:
	//	if((wParam & 0xFFF0) == SC_MINIMIZE)
	//	{
	//		ShowWindow(hWnd, SW_HIDE);
	//		return 1;
	//	}
	//	break;
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
	//case WM_INITDIALOG:
	//	return OnInitDialog(hWnd);
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		niData.uFlags = 0;
		KillTimer(hWnd, TIMER_ID);
		DestroyIcon(niData.hIcon);
		Shell_NotifyIcon(NIM_DELETE,&niData);
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		{
			static PerformanceMonitor pm;
			if (pm.next()) {
				DestroyIcon(niData.hIcon);
				niData.hIcon = pm.getIcon();
				pm.cpyTip(niData.szTip, sizeof(niData.szTip)/sizeof(TCHAR));
				niData.uFlags = NIF_ICON | NIF_TIP;
				if (!Shell_NotifyIcon(NIM_MODIFY, &niData)) {
					// This means the icon is disappeared because some reason like explorer had been killed.
					// So try to add the icon to taskbar again.
					niData.uFlags |= NIF_MESSAGE;
					Shell_NotifyIcon(NIM_ADD, &niData);
				}
				static int count = MIN_MEM_T;
				if (count++ > MIN_MEM_T) {
					::SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
					count %= MIN_MEM_T;
				}
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}
