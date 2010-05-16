// ScreenRuler.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "ScreenRuler.h"
#include "VisualRulerData.h"

#define MAX_LOADSTRING 100

#define COLOR_TRANS RGB(255, 255, 255)
#define NOFOCUS_TRANS_VALUE	128
#define FOCUS_TRANS_VALUE	255
#define MAX_COPY_LEN		128

// �O���[�o���ϐ�:
HINSTANCE hInst;								// ���݂̃C���^�[�t�F�C�X
TCHAR szTitle[MAX_LOADSTRING];					// �^�C�g�� �o�[�̃e�L�X�g
TCHAR szWindowClass[MAX_LOADSTRING];			// ���C�� �E�B���h�E �N���X��
HBRUSH transColorBrush;							// A brush to draw main window
VisualRulerData* vrd = NULL;					// Main data
TCHAR* szCopy = NULL;							// To store content to copy

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂�:
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

 	// TODO: �����ɃR�[�h��}�����Ă��������B
	MSG msg;
	HACCEL hAccelTable;

	// �O���[�o������������������Ă��܂��B
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SCREENRULER, szWindowClass, MAX_LOADSTRING);
	transColorBrush = CreateSolidBrush(COLOR_TRANS); 
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance (hInstance, SW_MAXIMIZE))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENRULER));

	// ���C�� ���b�Z�[�W ���[�v:
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
//  �֐�: MyRegisterClass()
//
//  �ړI: �E�B���h�E �N���X��o�^���܂��B
//
//  �R�����g:
//
//    ���̊֐�����юg�����́A'RegisterClassEx' �֐����ǉ����ꂽ
//    Windows 95 ���O�� Win32 �V�X�e���ƌ݊�������ꍇ�ɂ̂ݕK�v�ł��B
//    �A�v���P�[�V�������A�֘A�t����ꂽ
//    �������`���̏������A�C�R�����擾�ł���悤�ɂ���ɂ́A
//    ���̊֐����Ăяo���Ă��������B
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
//   �֐�: InitInstance(HINSTANCE, int)
//
//   �ړI: �C���X�^���X �n���h����ۑ����āA���C�� �E�B���h�E���쐬���܂��B
//
//   �R�����g:
//
//        ���̊֐��ŁA�O���[�o���ϐ��ŃC���X�^���X �n���h����ۑ����A
//        ���C�� �v���O���� �E�B���h�E���쐬����ѕ\�����܂��B
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

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
//  �֐�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  �ړI:  ���C�� �E�B���h�E�̃��b�Z�[�W���������܂��B
//
//  WM_COMMAND	- �A�v���P�[�V���� ���j���[�̏���
//  WM_PAINT	- ���C�� �E�B���h�E�̕`��
//  WM_DESTROY	- ���~���b�Z�[�W��\�����Ė߂�
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