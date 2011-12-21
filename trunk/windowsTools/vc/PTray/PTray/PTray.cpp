#include "stdafx.h"
#include "Psapi.h"
#include "resource.h"
#include "PerformanceMonitor.h"
#include "ProcessPriorityTable.h"

#define TRAYICONID	1//				ID number for the Notify Icon
#define TIMER_ID	1//				Timer ID

#define TIMER_DUR	500
#define SWM_TRAYMSG	WM_APP//		the message ID sent to our window
//#define MIN_MEM_T	5

#define SWM_SHOW	WM_APP + 1//	show the window
#define SWM_HIDE	WM_APP + 2//	hide the window
#define SWM_EXIT	WM_APP + 3//	close the window

#define APP_NAME	"Performance Tray"
#define MUTEX_SZ	"Performance System Tray. //"
#define PROC_MAX	1024
#define MAX_FPATH	1024
#define MAX_FNAME	256
#define NP_INDEX	2			// Normal Priority Index

typedef BOOL (*PROCFUNC)(DWORD);

// Global Variables:
HINSTANCE				hInst;		// current instance
NOTIFYICONDATA			niData;		// notify icon data
PerformanceMonitor		pm;			// performance monitor instance
ProcessPriorityTable	ppt;		// process priority bi-tree table
BOOL					bMsgBox;	// indicate whether the message box is being shown

const DWORD PCLASSES[] = {
	IDLE_PRIORITY_CLASS, 
	BELOW_NORMAL_PRIORITY_CLASS,
	NORMAL_PRIORITY_CLASS,
	ABOVE_NORMAL_PRIORITY_CLASS,
	HIGH_PRIORITY_CLASS,
	REALTIME_PRIORITY_CLASS,
};

BOOL				InitInstance(HINSTANCE, int);
BOOL				AddSysTray(HWND hWnd);
void				ProcessParameter(LPTSTR lpCmdLine);
INT_PTR CALLBACK	TrayProc(HWND, UINT, WPARAM, LPARAM);

void				LoadIniFile();

BOOL				GetDebugPrivilege();
void				FuncAllProcess(PROCFUNC);
BOOL				SweepProcess(DWORD processId);
BOOL				SetProcessPriority(DWORD processId);

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

	ProcessParameter(lpCmdLine);
	LoadIniFile();
	GetDebugPrivilege();

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;

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
	COLORREF cpuColor = CPU_COLOR;
	COLORREF memColor = MEM_COLOR;
	for (TCHAR* p = _tcstok_s(lpCmdLine, delimiter, &context); p; p = _tcstok_s(NULL, delimiter, &context)) {
		if (*p == _T('-')) {
			COLORREF* pColor = NULL;
			if (lstrcmpi(p, _T("-cpu")) == 0) {
				pColor = &cpuColor;
			}
			if (lstrcmpi(p, _T("-mem")) == 0 || lstrcmpi(p, _T("-memory")) == 0) {
				pColor = &memColor;
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
	pm.setColors(cpuColor, memColor);
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

	SetTimer(hWnd, TIMER_ID, TIMER_DUR, NULL);

	pm.next();

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

	switch (message) 
	{
	case SWM_TRAYMSG:
		switch(lParam)
		{
		case WM_LBUTTONDOWN:
			FuncAllProcess(SweepProcess);
			break;
		case WM_LBUTTONDBLCLK:
			{
				LoadIniFile();
				if (!bMsgBox) {
					bMsgBox = TRUE;
					MessageBox(hWnd, _T("Tiny tool developed by \n    Programus (programus@gmail.com)\n\n[** ini file reloaded **]"), _T(APP_NAME), MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST);
					bMsgBox = FALSE;
					// Minimize physical memory once. 
					::SetProcessWorkingSetSize(::GetCurrentProcess(), -1, -1);
				}
			}
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			{
				if (!bMsgBox) {
					bMsgBox = TRUE;
					if (MessageBox(hWnd, _T("Quit?"), _T(APP_NAME), MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST) == IDYES) {
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
		DestroyIcon(niData.hIcon);
		Shell_NotifyIcon(NIM_DELETE,&niData);
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		{
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
			}
			if (ppt.getProcessNum() && !ppt.bProcessing && !ppt.bInterrupted) {
				ppt.bProcessing = TRUE;
				FuncAllProcess(SetProcessPriority); 
				ppt.bProcessing = FALSE;
			}
			//static int count = MIN_MEM_T;
			//if (count++ > MIN_MEM_T) {
			//	::SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
			//	count %= MIN_MEM_T;
			//}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

// enumerate all processes and process it by using the specified function.
void FuncAllProcess(PROCFUNC func)
{
	DWORD processIds[PROC_MAX];
	DWORD cbNeeded = 0;
	DWORD nProcess = 0;

	if (::EnumProcesses(processIds, sizeof(processIds), &cbNeeded)) {
		nProcess = cbNeeded / sizeof(DWORD);
		for (unsigned int i = 0; i < nProcess; i++) {
			if (processIds[i]) {
				func(processIds[i]);
			}
		}
	}
}

BOOL SweepProcess(DWORD processId)
{
	BOOL ret = FALSE;
	if (processId) {
		HANDLE hProcess = ::OpenProcess(PROCESS_SET_QUOTA, FALSE, processId); 
		if (hProcess) {
			ret = ::SetProcessWorkingSetSize(hProcess, -1, -1);
		}
		::CloseHandle(hProcess);
		hProcess = NULL;
	}
	return ret;
}

BOOL SetProcessPriority(DWORD processId)
{
	BOOL ret = FALSE;
	if (processId && !ppt.bInterrupted) {
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION | PROCESS_VM_READ, FALSE, processId); 
		if (hProcess) {
			int priority = PRIORITY_UNCHANGE;
			TCHAR* fileName = (TCHAR*)calloc(MAX_FPATH, sizeof(TCHAR));
			DWORD len = ::GetModuleFileNameEx(hProcess, NULL, fileName, MAX_FPATH);
			if (len) {
				priority = ppt.getPriorityValue(fileName);
			}
			if (priority == PRIORITY_UNCHANGE) {
				memset(fileName, 0, MAX_FPATH);
				len = ::GetModuleBaseName(hProcess, NULL, fileName, MAX_FPATH);
				if (len) {
					priority = ppt.getPriorityValue(fileName);
				}
			}
			if (priority != PRIORITY_UNCHANGE) {
				DWORD targetPriority = PCLASSES[priority + NP_INDEX];
				DWORD currPriority = ::GetPriorityClass(hProcess);
				if (currPriority != targetPriority) {
					ret = ::SetPriorityClass(hProcess, targetPriority);
				}
			} else if (len) {
				ret = TRUE;
			}
			if (fileName) {
				free(fileName);
			}
		}
		::CloseHandle(hProcess);
		hProcess = NULL;
	}
	return ret;
}

BOOL GetDebugPrivilege()
{
    BOOL             fSuccess    = FALSE;
    HANDLE           TokenHandle = NULL;
    TOKEN_PRIVILEGES TokenPrivileges;

    if (OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &TokenHandle))
    {
		TokenPrivileges.PrivilegeCount = 1;

		if (LookupPrivilegeValue(NULL,
								  SE_DEBUG_NAME,
								  &TokenPrivileges.Privileges[0].Luid))
		{

			TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			if (AdjustTokenPrivileges(TokenHandle,
									   FALSE,
									   &TokenPrivileges,
									   sizeof(TokenPrivileges),
									   NULL,
									   NULL))
			{
				fSuccess = TRUE;
			}

		}
    }
    if (TokenHandle)
    {
        CloseHandle(TokenHandle);
    }
    return fSuccess;
}

void LoadIniFile()
{
	ppt.bInterrupted = TRUE;
	TCHAR* fileName = NULL;
	fileName = (TCHAR*)calloc(MAX_FPATH, sizeof(TCHAR));
	DWORD len = ::GetModuleFileName(NULL, fileName, MAX_FPATH);
	TCHAR* exPart = fileName + len - 4;
	if (lstrcmpi(exPart, _T(".exe")) == 0) {
		lstrcpyn(exPart, _T(".ini"), 5);
		ppt.clear();
		ppt.loadFromIniFile(fileName);
	}
	if (fileName) {
		free(fileName);
		fileName = NULL;
		exPart = NULL;
	}
	ppt.bInterrupted = FALSE;
}
