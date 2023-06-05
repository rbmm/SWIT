#include "stdafx.h"
#include "resource.h"
#include "UiContext.h"

BOOL MoveWndTo(HWND hwnd, int x, int y)
{
	return SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOREDRAW|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}

class QrWnd : public CDialogImpl<QrWnd>
{
	class CSubWnd : public CWindowImplBaseT<>
	{
		void OnPosChanged(WINDOWPOS* pwp)
		{
			CONTAINING_RECORD(this, QrWnd, _M_subWnd)->OnPosChanged(pwp->x, pwp->y, pwp->cx);
		}

		virtual BOOL ProcessWindowMessage(
			_In_ HWND /*hwnd*/,
			_In_ UINT uMsg,
			_In_ WPARAM /*wParam*/,
			_In_ LPARAM lParam,
			_Inout_ LRESULT& /*lResult*/,
			_In_ DWORD /*dwMsgMapID*/)
		{
			switch (uMsg)
			{
			case WM_NCDESTROY:
				UnsubclassWindow(TRUE);
				break;

			case WM_WINDOWPOSCHANGED:
				OnPosChanged(reinterpret_cast<WINDOWPOS*>(lParam));
				break;
			}

			return FALSE;
		}
	};

	CSubWnd _M_subWnd;
	HCURSOR _M_hc = 0;
	HWND _M_hwnd = 0;
	PPOINT _M_ppt;
	int _M_x = 0, _M_y = 0;

	void OnDestroy(HWND hwnd)
	{
		if (_M_hc) DestroyCursor(_M_hc);
		RECT rc;
		if (::GetWindowRect(hwnd, &rc))
		{
			_M_ppt->x = rc.left;
			_M_ppt->y = rc.top;
		}

		_M_subWnd.UnsubclassWindow();
	}

	virtual void OnFinalMessage(_In_ HWND /*hWnd*/)
	{
		delete this;
	}

	void OnPosChanged(int x, int y, int cx)
	{
		if (x != _M_x || y != _M_y)
		{
			_M_y = y, _M_x = x;
			MoveWndTo(m_hWnd, x + cx, y);
		}
	}

	virtual BOOL ProcessWindowMessage(
		_In_ HWND hwndDlg,
		_In_ UINT uMsg,
		_In_ WPARAM /*wParam*/,
		_In_ LPARAM lParam,
		_Inout_ LRESULT& lResult,
		_In_ DWORD /*dwMsgMapID*/)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
			lResult = HTCAPTION;
			return TRUE;

		case WM_SETCURSOR:
			SetCursor(_M_hc);
			lResult = TRUE;
			return TRUE;

		case WM_INITDIALOG:
			*(BOOL*)lParam = TRUE;
			_M_hc = LoadCursor(0, IDC_SIZEALL);
			break;

		case WM_CLOSE:
			::DestroyWindow(hwndDlg);
			break;

		case WM_NCDESTROY:
			OnDestroy(hwndDlg);
			break;
		}

		return FALSE;
	}

public:
	enum { IDD = IDD_DIALOG1 };

	BOOL SetSubclass(_In_ HWND hwnd)
	{
		RECT rc;
		if (::GetWindowRect(hwnd, &rc))
		{
			_M_x = rc.left, _M_y = rc.top;
			
			return _M_subWnd.SubclassWindow(hwnd);
		}

		return FALSE;
	}

	QrWnd(PPOINT ppt) : _M_ppt(ppt)
	{
	}
};
 
HWND GetMainWnd(ULONG dwThreadId)
{
	ULONG n = 10;

	GUITHREADINFO gti = { sizeof(gti) };
	do 
	{
		Sleep(100);
	} while (GetGUIThreadInfo(dwThreadId, &gti) && !gti.hwndActive && --n);

	return gti.hwndActive;
}

class ProvDlg : public CDialogImpl<ProvDlg>
{
	HWND _M_hwndMain = 0;
	HWND _M_hwndQR = 0;
	POINT _M_pt = { CW_USEDEFAULT, CW_USEDEFAULT };

	HWND execute(HWND hwnd)
	{
		if (QrWnd* p = new QrWnd(&_M_pt))
		{
			BOOL bCalled = FALSE;

			if (HWND hwndMy = p->Create(HWND_DESKTOP, (LPARAM)&bCalled))
			{
				if (0 <= _M_pt.x && 0 <= _M_pt.y)
				{
					MoveWndTo(hwndMy, _M_pt.x, _M_pt.y);
				}

				p->SetSubclass(hwnd);

				return hwndMy;
			}

			if (!bCalled) delete this;
		}

		return 0;
	}

	static PVOID WINAPI _S_execute(HWND hwnd, PVOID This)
	{
		return reinterpret_cast<ProvDlg*>(This)->execute(hwnd);
	}
	
	HWND ShowQr()
	{
		return (HWND)invoke_in_ui(_M_hwndMain, _S_execute, this);
	}

	void HideQr(HWND hwndDlg)
	{
		if (HWND hwndQR = _M_hwndQR)
		{
			::SendMessageW(hwndQR, WM_CLOSE, 0, 0);
			SetState(hwndDlg, 0);
		}
	}

	void SetState(HWND hwndDlg, HWND hwndQR)
	{
		_M_hwndQR = hwndQR;
		::EnableWindow(::GetDlgItem(hwndDlg, IDOK), hwndQR == 0);
		::EnableWindow(::GetDlgItem(hwndDlg, IDCANCEL), hwndQR != 0);
	}

	void InitPos(HWND hwndDlg)
	{
		RECT rc;
		if (::GetWindowRect(hwndDlg, &rc))
		{
			ULONG dx = rc.right - rc.left;
			if (::GetWindowRect(_M_hwndMain, &rc))
			{
				MoveWndTo(hwndDlg, rc.left - dx, rc.top);
				_M_pt.x = rc.right;
				_M_pt.y = rc.top;
			}
		}
	}

	virtual BOOL ProcessWindowMessage(
		_In_ HWND hwndDlg,
		_In_ UINT uMsg,
		_In_ WPARAM wParam,
		_In_ LPARAM /*lParam*/,
		_Inout_ LRESULT& lResult,
		_In_ DWORD /*dwMsgMapID*/)
	{
		lResult = FALSE;

		switch (uMsg)
		{
		case WM_INITDIALOG:
			InitPos(hwndDlg);
__ok:
			SetState(hwndDlg, ShowQr());
			break;

		case WM_CLOSE:
			::DestroyWindow(hwndDlg);
			break;

		case WM_NCDESTROY:
__cancel:
			HideQr(hwndDlg);
			break;

		case WM_COMMAND:
			switch (wParam)
			{
			case IDOK:
				goto __ok;
			case IDCANCEL:
				goto __cancel;
			}
			break;
		}

		return FALSE;
	}

public:
	enum { IDD = IDD_DIALOG2 };

	BOOL Init(ULONG dwThreadId)
	{
		if (HWND hwndMain = GetMainWnd(dwThreadId))
		{
			_M_hwndMain = hwndMain;

			return TRUE;
		}

		return FALSE;
	}
};

ULONG ProviderThread(PVOID dwThreadId)
{
	ProvDlg dlg;

	if (dlg.Init((ULONG)(ULONG_PTR)dwThreadId))
	{
		dlg.DoModal(HWND_DESKTOP);
	}

	return 0;
}

#include "../inc/initterm.h"

void WINAPI ep(HANDLE hThread)
{
	initterm();

	ULONG dwThreadId;
	if (hThread = CreateThread(0, 0, ProviderThread, (PVOID)(ULONG_PTR)GetCurrentThreadId(), 0, &dwThreadId))
	{
		MessageBox(0, 0, L"MainWnd", MB_ICONINFORMATION);
		PostThreadMessageW(dwThreadId, WM_QUIT, 0, 0);
		
__loop:
		switch (MsgWaitForMultipleObjectsEx(1, &hThread, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE))
		{
		case WAIT_OBJECT_0:
			break;
		case WAIT_OBJECT_0 + 1:
			MSG msg;
			while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
			{
				DispatchMessageW(&msg);
			}
			goto __loop;
		default:
			__debugbreak();
		}
		
		CloseHandle(hThread);
	}

	destroyterm();

	ExitProcess(0);
}