#include "stdafx.h"
#include "MessageProcess.h"
#include "Resource.h"
#include "TinyCountdown.h"
#include "Countdown.h"
#include <tchar.h>

#define MAX_TIME_LEN	9

// �O���[�o���ϐ�:
extern HINSTANCE hInst;								// ���݂̃C���^�[�t�F�C�X
extern TCHAR szTitle[];					// �^�C�g�� �o�[�̃e�L�X�g
extern TCHAR szWindowClass[];			// ���C�� �E�B���h�E �N���X��

Countdown cd; 

void SetTime(HWND hWnd)
{
	UINT state = cd.getState();
	if (!(state & CDS_PAUSED))
	{
		cd.pauseCountdown();
	}
	if (DialogBox(hInst, MAKEINTRESOURCE(IDD_TIME_INPUT), hWnd, TimeInputProc) == IDOK)
	{
		cd.resetCountdown(hWnd); 
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE); 
	}
	else if (!(state & CDS_PAUSED))
	{
		cd.pauseCountdown();
	}
}

void PauseStartCountdown(HWND hWnd)
{
	UINT state = cd.getState();
	switch(state)
	{
	case CDS_STOPPED:
		cd.resetCountdown(hWnd);
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE); 
		break;
	case CDS_WAITING:
		cd.startCountdown(hWnd);
		break;
	default:
		cd.pauseCountdown();
		break;
	}
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
	int wmId, wmEvent;
	static POINT preMousePos; 
	static RECT preWindowPos;
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �I�����ꂽ���j���[�̉��:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			if (cd.getState() & CDS_STARTED)
			{
				if (MessageBox(hWnd, _T("Countdown is ongoing. Are you sure you want to exit?"), _T("Question"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					DestroyWindow(hWnd);
				}
			}
			else
			{
				DestroyWindow(hWnd);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_TIMER:
		cd.TimerProc(hWnd);
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE); 
		break;
	case WM_PAINT:
		cd.draw(hWnd); 
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_RBUTTONUP:
		SetTime(hWnd);
		break;
	case WM_LBUTTONDOWN:
		{
			SetCapture(hWnd); 
			preMousePos.x = GET_X_LPARAM(lParam); 
			preMousePos.y = GET_Y_LPARAM(lParam);
			ClientToScreen(hWnd, &preMousePos);
			GetWindowRect(hWnd, &preWindowPos); 
		}
		break;
	case WM_LBUTTONUP:
		{
			RECT r;
			GetWindowRect(hWnd, &r);
			if (!memcmp(&preWindowPos, &r, sizeof(RECT)))
			{
				PauseStartCountdown(hWnd);
			}
			ReleaseCapture();
		}
		break;
	case WM_MOUSEMOVE:
		{
			if (GetCapture() == hWnd)
			{
				POINT p;
				p.x = GET_X_LPARAM(lParam); 
				p.y = GET_Y_LPARAM(lParam);
				ClientToScreen(hWnd, &p);
				int dx = p.x - preMousePos.x; 
				int dy = p.y - preMousePos.y; 
				MoveWindow(hWnd, 
					preWindowPos.left + dx, 
					preWindowPos.top + dy, 
					preWindowPos.right - preWindowPos.left, 
					preWindowPos.bottom - preWindowPos.top, 
					TRUE); 
			}
		}
		break;
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_RETURN:
			SetTime(hWnd);
			break;
		case VK_SPACE:
			PauseStartCountdown(hWnd);
			break;
		case VK_BACK:
			cd.resetCountdown(hWnd);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE); 
			break;
		case VK_HOME:
			SetTopMost(hWnd, TRUE); 
			break;
		case VK_END:
			SetTopMost(hWnd, FALSE);
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK TimeInputProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_TIME);
			Edit_LimitText(hEdit, MAX_TIME_LEN);

			int sec = cd.getTotalTime();
			int hour = sec / 3600;
			int min = sec / 60;
			sec %= 60;
			min %= 60;
			TCHAR time[MAX_TIME_LEN + 1];
			_stprintf(time, _T("%d:%02d:%02d"), hour, min, sec); 
			Edit_SetText(hEdit, time); 
			Edit_SetSel(hEdit, 0, MAX_TIME_LEN + 1);

			if (GetDlgCtrlID((HWND) wParam) != IDC_EDIT_TIME)
			{
				SetFocus(hEdit); 
				return FALSE;
			}
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				int time[3] = {0, -1, -1};
				HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_TIME);
				TCHAR editText[MAX_TIME_LEN + 1];
				int len = Edit_GetText(hEdit, editText, MAX_TIME_LEN + 1);
				for (int i = 0, j = 0; i < len; i++)
				{
					if (editText[i] != ':')
					{
						time[j] *= 10;
						time[j] += (editText[i] - '0');
					}
					else
					{
						time[++j] = 0;
					}
				}
				UINT sec = time[0];
				for (int i = 1; i < 3; i++)
				{
					if (time[i] >= 0)
					{
						sec *= 60;
						sec += time[i];
					}
				}
				cd.setTotalTime(sec);
			}
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		case IDC_EDIT_TIME:
			{
				switch(HIWORD(wParam))
				{
				case EN_UPDATE:
					{
						static BOOL changing = FALSE;
						if (!changing)
						{
							HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_TIME); 
							TCHAR editText[MAX_TIME_LEN + 1];
							TCHAR finalText[MAX_TIME_LEN + 1];
							DWORD sel = Edit_GetSel(hEdit);
							int len = Edit_GetText(hEdit, editText, MAX_TIME_LEN + 1);
							ZeroMemory(finalText, sizeof(finalText));
							int colonCount = 0;
							for (int i = 0, j = 0; i < len; i++)
							{
								if ((editText[i] >= '0' && editText[i] <= '9') || (editText[i] == ':' && colonCount < 2))
								{
									finalText[j++] = editText[i]; 
									if (editText[i] == ':') colonCount++;
								}
							}
							changing = TRUE;
							Edit_SetText(hEdit, finalText);
							Edit_SetSel(hEdit, LOWORD(sel), HIWORD(sel));
							changing = FALSE;
						}
					}
					break;
				}
			}
			break;
		}
		break;

	}
	return (INT_PTR)FALSE;
}
