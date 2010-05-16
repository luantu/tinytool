// ScreenRuler.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "ScreenRuler.h"
#include "VisualRulerData.h"

#define MAX_LOADSTRING 100

#define COLOR_TRANS RGB(255, 255, 255)
#define NOFOCUS_TRANS_VALUE	128
#define FOCUS_TRANS_VALUE	255
#define MAX_COPY_LEN		128

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
   ::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED); 

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
	static int draggingPointFlag = 0; 

	switch (message)
	{
	case WM_CREATE:
		{
			// Set WS_EX_LAYERED on this window 
			SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED/* | WS_EX_TOOLWINDOW*/);
			// Make this window alpha
			SetTransValue(hWnd, FOCUS_TRANS_VALUE); 
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
			MinimizeMemory(); 
		}
		break; 
	case WM_LBUTTONDOWN:
		{
			::SetCapture(hWnd); 
			RECT rect; 
			::GetWindowRect(hWnd, &rect); 
			::ClipCursor(&rect); 
			POINT mousePt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; 
			oldMousePt = mousePt; 
			oldStartPt = vrd->startPt; 
			oldEndPt = vrd->endPt; 
			if (vrd->IsPointIntoEndPoint(&mousePt))
			{
				draggingPointFlag = 2; 
			}
			else if (vrd->IsPointIntoStartPoint(&mousePt))
			{
				draggingPointFlag = 1; 
			}
			else
			{
				draggingPointFlag = 3; 
			}

		}
		break; 
	case WM_LBUTTONUP:
		{
			vrd->AdjustLabelOrientation(); 
			vrd->AdjustLineLabelOrientation(); 
			ClipCursor(NULL); 
			::RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW); 
			::ReleaseCapture(); 
			MinimizeMemory(); 
		}
		break;
	case WM_MOUSEMOVE:
		{
			if (wParam & MK_LBUTTON)
			{
				POINT mousePt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; 
				int dx = mousePt.x - oldMousePt.x; 
				int dy = mousePt.y - oldMousePt.y; 
				if (draggingPointFlag & 1)
				{
					vrd->startPt.x = oldStartPt.x + dx; 
					vrd->startPt.y = oldStartPt.y + dy; 
				}
				if (draggingPointFlag & 2)
				{
					vrd->endPt.x = oldEndPt.x + dx; 
					vrd->endPt.y = oldEndPt.y + dy; 
				}
				if (draggingPointFlag != 3)
				{
					vrd->AdjustLineLabelOrientation(); 
				}
				::RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW); 
			}
		}
		break; 
	case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				EndInstance(); 
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

				if (vrd->IsPointIntoEndPoint(&pt))
				{
					vrd->GetEndPoint(&szCopy, MAX_COPY_LEN); 
				}
				else if (vrd->IsPointIntoStartPoint(&pt))
				{
					vrd->GetStartPoint(&szCopy, MAX_COPY_LEN); 
				}
				else
				{
					vrd->GetDistance(&szCopy, MAX_COPY_LEN); 
				}

				int l = MAX_COPY_LEN + 16; 
				TCHAR* szText = new TCHAR[l]; 
				LoadString(hInst, IDS_COPY, szText, l); 
				_tcscat_s(szText, l, szCopy); 

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
	case WM_SETFOCUS:
		SetTransValue(hWnd, FOCUS_TRANS_VALUE); 
		break; 
	case WM_KILLFOCUS:
		SetTransValue(hWnd, NOFOCUS_TRANS_VALUE); 
		break; 
	case WM_DESTROY:
		EndInstance(); 
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
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