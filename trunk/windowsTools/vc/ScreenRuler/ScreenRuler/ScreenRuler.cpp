// ScreenRuler.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "ScreenRuler.h"
#include "VisualRulerData.h"

#define MAX_LOADSTRING 100

#define COLOR_TRANS			RGB(255, 255, 255)
#define NOFOCUS_TRANS_VALUE	128
#define FOCUS_TRANS_VALUE	255
#define MAX_COPY_LEN		128

#define IDT_KEY_DETECT		1
#define KDT_DELAY			64

// グローバル変数:
HINSTANCE hInst;								// 現在のインターフェイス
TCHAR szTitle[MAX_LOADSTRING];					// タイトル バーのテキスト
TCHAR szWindowClass[MAX_LOADSTRING];			// メイン ウィンドウ クラス名
HBRUSH transColorBrush;							// A brush to draw main window
VisualRulerData* vrd = NULL;					// Main data
TCHAR* szCopy = NULL;							// To store content to copy

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: ここにコードを挿入してください。
	MSG msg;
	HACCEL hAccelTable;

	// グローバル文字列を初期化しています。
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SCREENRULER, szWindowClass, MAX_LOADSTRING);
	transColorBrush = CreateSolidBrush(COLOR_TRANS); 
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, SW_MAXIMIZE))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENRULER));

	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
//  コメント:
//
//    この関数および使い方は、'RegisterClassEx' 関数が追加された
//    Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
//    アプリケーションが、関連付けられた
//    正しい形式の小さいアイコンを取得できるようにするには、
//    この関数を呼び出してください。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREENRULER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= transColorBrush; //(HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= NULL; //MAKEINTRESOURCE(IDC_SCREENRULER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SCREENRULER));

	return RegisterClassEx(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_SYSMENU,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   SetTopMost(hWnd); 

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // My own initialize codes. 
   vrd = new VisualRulerData(hWnd); 
   RECT rect; 
   ::GetWindowRect(hWnd, &rect); 
   int mx = rect.right >> 1; 
   int my = rect.bottom >> 1; 
   vrd->startPt.x = rect.left + (mx >> 1); 
   vrd->startPt.y = rect.top + (my >> 1); 
   vrd->endPt.x = rect.right - (mx >> 1); 
   vrd->endPt.y = rect.bottom - (mx >> 1); 
   vrd->AdjustLabelOrientation(); 
   vrd->AdjustLineLabelOrientation(); 
   szCopy = new TCHAR[MAX_COPY_LEN]; 
   MinimizeMemory(); 
   // End own init


   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static POINT oldMousePt; 
	static POINT oldStartPt; 
	static POINT oldEndPt; 

	static int pressedArrowKeys = 0; // Flag Bits: |     |     |     |Mouse|Left|Up|Right|Down|
	static int oldFocusPointFlag = FPF_BOTH; 
	static LONG lastTickCount = 0; 

	BOOL bNeedRedraw = FALSE; 
	BOOL bNeedMinimizeMemory = FALSE; 

	switch (message)
	{
	case WM_CREATE:
		{
			// Set WS_EX_LAYERED on this window 
			SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED/* | WS_EX_TOOLWINDOW*/);
			// Make this window alpha
			SetTransValue(hWnd, FOCUS_TRANS_VALUE); 
			bNeedMinimizeMemory = TRUE; 
		}
		break; 
	case WM_PAINT:
		{
			if (vrd != NULL)
			{
				vrd->Draw(); 
			}
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDM_EXIT:
				EndInstance(); 
				break; 
			case IDM_COPY:
				{
					if (::OpenClipboard(hWnd))
					{
						switch(vrd->m_focusPointFlag)
						{
						case FPF_END:
							vrd->GetEndPoint(&szCopy, MAX_COPY_LEN); 
							break; 
						case FPF_START:
							vrd->GetStartPoint(&szCopy, MAX_COPY_LEN); 
							break; 
						default:
							vrd->GetDistance(&szCopy, MAX_COPY_LEN); 
							break; 
						}
						EmptyClipboard(); 
						size_t len = _tcslen(szCopy); 
						HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(TCHAR));
						if (hglbCopy != NULL)
						{
							LPTSTR lptstrCopy = (LPTSTR) GlobalLock(hglbCopy); 
							memcpy(lptstrCopy, szCopy, len * sizeof(TCHAR)); 
							lptstrCopy[len] = '\0'; 
							GlobalUnlock(hglbCopy); 

							::SetClipboardData(CF_UNICODETEXT, hglbCopy); 
						}
						CloseClipboard(); 
					}
				}
				break;
			case IDM_CROSS:
				vrd->m_showCross = !vrd->m_showCross; 
				bNeedRedraw = TRUE; 
				break; 
			case IDM_ABOUT:
				{
					TCHAR* szAbout = new TCHAR[256]; 
					::LoadString(hInst, IDS_ABOUT, szAbout, 256); 
					TCHAR* szTitle = new TCHAR[32]; 
					::LoadString(hInst, IDS_ABOUT_TITLE, szTitle, 32); 
					::MessageBox(hWnd, szAbout, szTitle, MB_OK | MB_ICONINFORMATION); 
				}
				break; 
			}
			bNeedMinimizeMemory = TRUE; 
		}
		break; 
	case WM_LBUTTONDOWN:
		{
			::SetCapture(hWnd); 
			RECT rect; 
			::GetWindowRect(hWnd, &rect); 
			::ClipCursor(&rect); 
			pressedArrowKeys |= (1 << 4); 
			POINT mousePt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; 
			oldMousePt = mousePt; 
			oldStartPt = vrd->startPt; 
			oldEndPt = vrd->endPt; 
			if (vrd->IsPointIntoEndPoint(&mousePt))
			{
				bNeedRedraw = vrd->m_focusPointFlag != FPF_END; 
				vrd->m_focusPointFlag = FPF_END;
			}
			else if (vrd->IsPointIntoStartPoint(&mousePt))
			{
				bNeedRedraw = vrd->m_focusPointFlag != FPF_START; 
				vrd->m_focusPointFlag = FPF_START;
			}
			else
			{
				bNeedRedraw = vrd->m_focusPointFlag != FPF_BOTH; 
				vrd->m_focusPointFlag = FPF_BOTH;
			}
		}
		break; 
	case WM_LBUTTONUP:
		{
			pressedArrowKeys &= ~(1 << 4); 
			if (pressedArrowKeys == 0)
			{
				vrd->AdjustLabelOrientation(); 
				vrd->AdjustLineLabelOrientation(); 
			}
			ClipCursor(NULL); 
			::RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW); 
			::ReleaseCapture(); 
			bNeedMinimizeMemory = TRUE; 
		}
		break;
	case WM_MOUSEMOVE:
		{
			if (wParam & MK_LBUTTON)
			{
				POINT mousePt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; 
				int dx = mousePt.x - oldMousePt.x; 
				int dy = mousePt.y - oldMousePt.y; 
				CrossMove(VK_SHIFT, &dx, &dy); 
				if (vrd->m_focusPointFlag & 1)
				{
					vrd->startPt.x = oldStartPt.x + dx; 
					vrd->startPt.y = oldStartPt.y + dy; 
				}
				if (vrd->m_focusPointFlag & 2)
				{
					vrd->endPt.x = oldEndPt.x + dx; 
					vrd->endPt.y = oldEndPt.y + dy; 
				}
				if (vrd->m_focusPointFlag != 3)
				{
					vrd->AdjustLineLabelOrientation(); 
				}
				::RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW); 
			}
		}
		break; 
	case WM_RBUTTONDOWN:
		{
			POINT mousePt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; 
			if (vrd->IsPointIntoEndPoint(&mousePt))
			{
				bNeedRedraw = vrd->m_focusPointFlag != FPF_END; 
				vrd->m_focusPointFlag = FPF_END;
			}
			else if (vrd->IsPointIntoStartPoint(&mousePt))
			{
				bNeedRedraw = vrd->m_focusPointFlag != FPF_START; 
				vrd->m_focusPointFlag = FPF_START;
			}
			else
			{
				bNeedRedraw = vrd->m_focusPointFlag != FPF_BOTH; 
				vrd->m_focusPointFlag = FPF_BOTH;
			}
		}
		break; 
	case WM_CONTEXTMENU:
		{
			POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; 
			HMENU hMenu = ::LoadMenu(hInst, MAKEINTRESOURCE(IDC_SCREENRULER)); 
			if (hMenu != NULL)
			{
				HMENU hPopupMenu = ::GetSubMenu(hMenu, 0); 

				switch(vrd->m_focusPointFlag)
				{
				case FPF_END:
					vrd->GetEndPoint(&szCopy, MAX_COPY_LEN); 
					break; 
				case FPF_START:
					vrd->GetStartPoint(&szCopy, MAX_COPY_LEN); 
					break; 
				default:
					vrd->GetDistance(&szCopy, MAX_COPY_LEN); 
					break; 
				}

				int l = MAX_COPY_LEN + 16; 
				TCHAR* szText = new TCHAR[l]; 
				LoadString(hInst, IDS_COPY, szText, l); 
				_tcscat_s(szText, l, szCopy); 
				::CheckMenuItem(hMenu, IDM_CROSS, MF_BYCOMMAND | (vrd->m_showCross ? MF_CHECKED : MF_UNCHECKED)); 

				MENUITEMINFO* pmiiCopy = new MENUITEMINFO; 
				ZeroMemory(pmiiCopy, sizeof(*pmiiCopy)); 
				pmiiCopy->cbSize = sizeof(*pmiiCopy); 
				pmiiCopy->fMask = MIIM_STRING; 
				pmiiCopy->dwTypeData = szText; 
				pmiiCopy->cch = (UINT) _tcslen(szText); 
				if (::SetMenuItemInfo(hPopupMenu, IDM_COPY, FALSE, pmiiCopy))
				{
					DrawMenuBar(hWnd); 
				}
				::TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL); 
				DestroyMenu(hMenu); 
				delete[] szText; 
			}
		}
		break; 
	case WM_KEYDOWN:
		{
			switch(wParam)
			{
			case VK_ESCAPE:
				// ESC Key - Quit
				EndInstance(); 
				break;
			case '0':
			case '3':
			case 'R':
			case VK_NUMPAD0:
			case VK_NUMPAD3:
			case VK_RETURN:
				// 0 or 3 - Select both as moving focus point
				bNeedRedraw = vrd->m_focusPointFlag != FPF_BOTH; 
				vrd->m_focusPointFlag = FPF_BOTH;
				break; 
			case '1':
			case 'Q':
			case VK_NUMPAD1:
				// 1 - Select start point as moving focus point
				bNeedRedraw = vrd->m_focusPointFlag != FPF_START; 
				vrd->m_focusPointFlag = FPF_START;
				break; 
			case '2':
			case 'E':
			case VK_NUMPAD2:
				// 2 - Select end point as moving focus point
				bNeedRedraw = vrd->m_focusPointFlag != FPF_END; 
				vrd->m_focusPointFlag = FPF_END;
				break; 
			case VK_TAB:
				// switch points
				bNeedRedraw = TRUE; 
				vrd->m_focusPointFlag ^= FPF_BOTH; 
				if (vrd->m_focusPointFlag == FPF_BOTH || vrd->m_focusPointFlag == FPF_NONE)
				{
					vrd->m_focusPointFlag = FPF_START; 
				}
				break; 
			case 'W':
			case 'S':
			case 'A':
			case 'D':
				switch(wParam)
				{
				case 'W':
					wParam = VK_UP;
					break;
				case 'S':
					wParam = VK_DOWN;
					break;
				case 'A':
					wParam = VK_LEFT;
					break;
				case 'D':
					wParam = VK_RIGHT;
					break; 
				}
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				{
					if (pressedArrowKeys == 0) 
					{
						::SetTimer(hWnd, IDT_KEY_DETECT, KDT_DELAY, NULL); 
						::PostMessage(hWnd, WM_TIMER, IDT_KEY_DETECT, 0); 
					}
					pressedArrowKeys |= (1 << (wParam - VK_LEFT)); 
				}
				break; 
			}
		}
		break; 
	case WM_KEYUP:
		{
			switch(wParam)
			{
			case 'W':
			case 'S':
			case 'A':
			case 'D':
				switch(wParam)
				{
				case 'W':
					wParam = VK_UP;
					break;
				case 'S':
					wParam = VK_DOWN;
					break;
				case 'A':
					wParam = VK_LEFT;
					break;
				case 'D':
					wParam = VK_RIGHT;
					break; 
				}
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				{
					pressedArrowKeys &= ~(1 << (wParam - VK_LEFT)); 
					if (pressedArrowKeys == 0)
					{
						::KillTimer(hWnd, IDT_KEY_DETECT); 
						vrd->AdjustLabelOrientation(); 
						vrd->AdjustLineLabelOrientation(); 
						bNeedRedraw = TRUE; 
					}
				}
				break; 
			}
		}
		break;
	case WM_TIMER:
		{
			LONG tickCount = ::GetTickCount(); 
			if (tickCount - lastTickCount >= KDT_DELAY)
			{
				switch(wParam)
				{
				case IDT_KEY_DETECT:
					{
						int dx = 0; 
						int dy = 0; 
						if ((::GetAsyncKeyState(VK_LEFT) & 0x8000) || (::GetAsyncKeyState('A') & 0x8000))
						{
							dx -= 1; 
						}
						if ((::GetAsyncKeyState(VK_RIGHT) & 0x8000) || (::GetAsyncKeyState('D') & 0x8000))
						{
							dx += 1; 
						}
						if ((::GetAsyncKeyState(VK_UP) & 0x8000) || (::GetAsyncKeyState('W') & 0x8000))
						{
							dy -= 1; 
						}
						if ((::GetAsyncKeyState(VK_DOWN) & 0x8000) || (::GetAsyncKeyState('S') & 0x8000))
						{
							dy += 1; 
						}
						dx = dx > 0 ? 1 : (dx == 0 ? 0 : -1); 
						dy = dy > 0 ? 1 : (dy == 0 ? 0 : -1); 
						if ((::GetAsyncKeyState(VK_SHIFT) & 0x8000))
						{
							dx <<= 3; 
							dy <<= 3;
						}
						if ((::GetAsyncKeyState(VK_CONTROL) & 0x8000))
						{
							dx <<= 6; 
							dy <<= 6;
						}
						if (vrd->m_focusPointFlag & 1)
						{
							vrd->startPt.x += dx; 
							vrd->startPt.y += dy; 
						}
						if (vrd->m_focusPointFlag & 2)
						{
							vrd->endPt.x += dx; 
							vrd->endPt.y += dy;
						}
						if (vrd->m_focusPointFlag != 3)
						{
							vrd->AdjustLineLabelOrientation(); 
						}
						bNeedRedraw = (dx != 0 || dy != 0); 
					}
					break; 
				}
			}
			lastTickCount = tickCount; 
		}
		break;
	case WM_SETFOCUS:
		if (vrd != NULL)
		{
			vrd->m_focusPointFlag = oldFocusPointFlag; 
			bNeedRedraw = oldFocusPointFlag != FPF_BOTH; 
		}
		SetTopMost(hWnd); 
		SetTransValue(hWnd, FOCUS_TRANS_VALUE); 
		bNeedMinimizeMemory = TRUE; 
		break; 
	case WM_KILLFOCUS:
		if (vrd != NULL)
		{
			pressedArrowKeys = 0; 
			oldFocusPointFlag = vrd->m_focusPointFlag; 
			vrd->m_focusPointFlag = FPF_NONE; 
			bNeedRedraw = oldFocusPointFlag != FPF_BOTH; 
		}
		SetTopMost(hWnd); 
		SetTransValue(hWnd, NOFOCUS_TRANS_VALUE); 
		bNeedMinimizeMemory = TRUE; 
		break; 
	case WM_DESTROY:
		EndInstance(); 
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	if (bNeedRedraw)
	{
		::RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW); 
	}
	if (bNeedMinimizeMemory || bNeedRedraw)
	{
		MinimizeMemory(); 
	}

	return 0;
}

void CrossMove(__in int vKey, __inout int* dx, __inout int* dy)
{
	if (GetAsyncKeyState(vKey))
	{
		int adx = abs(*dx); 
		int ady = abs(*dy); 
		if (adx > ady)
		{
			*dy = 0; 
		}
		else if (ady > adx)
		{
			*dx = 0; 
		}
	}
}

void EndInstance()
{
	PostQuitMessage(0); 
	DeleteObject(transColorBrush); 
	delete[] szCopy; 
}

void MinimizeMemory()
{
	HANDLE hProcess = ::GetCurrentProcess(); 
	::SetProcessWorkingSetSize(hProcess, -1, -1); 
}

BOOL SetTransValue(HWND hWnd, BYTE transValue)
{
	DWORD flag = LWA_COLORKEY; 
	if (transValue < 255 && transValue > 0)
	{
		flag |= LWA_ALPHA; 
	}
	return SetLayeredWindowAttributes(hWnd, COLOR_TRANS, transValue, flag);
}

void SetTopMost(__in HWND hWnd)
{
	::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOACTIVATE); 
}