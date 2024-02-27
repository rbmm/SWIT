#include "stdafx.h"
#include "resource.h"
#include "UiContext.h"
#include "Subclass.h"
#include "dlg.h"

BOOL MoveWndTo(HWND hwnd, int x, int y)
{
	return SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOREDRAW|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}

class QrWnd : public CDlgBase, public SubClsWnd
{
	HCURSOR _M_hc = 0;
	HWND _M_hwnd = 0, m_hWnd = 0, _M_hwndWorker;
	PPOINT _M_ppt;
	int _M_x = 0, _M_y = 0;

	void OnDestroy(HWND hwnd)
	{
		if (_M_hc) DestroyCursor(_M_hc);

		RECT rc;

		if (GetWindowRect(hwnd, &rc))
		{
			_M_ppt->x = rc.left;
			_M_ppt->y = rc.top;
		}

		RemoveSubclass();
	}

	void OnPosChanged(int x, int y, int cx)
	{
		if (x != _M_x || y != _M_y)
		{
			_M_y = y, _M_x = x;
			MoveWndTo(m_hWnd, x + cx, y);
		}
	}

	virtual INT_PTR DlgProc(HWND hwnd, UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, HTCAPTION);
			return TRUE;

		case WM_SETCURSOR:
			SetCursor(_M_hc);
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
			return TRUE;

		case WM_INITDIALOG:
			_M_hc = LoadCursor(0, IDC_SIZEALL);
			m_hWnd = hwnd;
			return TRUE;

		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;

		case WM_NCDESTROY:
			OnDestroy(hwnd);
			OnNcDestroy();
			PostMessageW(_M_hwndWorker, WM_APP, 0, 0);
			break;
		}

		return 0;
	}

	virtual LRESULT CALLBACK MySubclassProc(HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass
		)
	{
		if (uIdSubclass != (UINT_PTR)&m_hWnd)
		{
			__debugbreak();
		}

		switch (uMsg)
		{
		case WM_NCDESTROY:
			RemoveSubclass();
			break;

		case WM_WINDOWPOSCHANGED:
			int x = reinterpret_cast<WINDOWPOS*>(lParam)->x, y = reinterpret_cast<WINDOWPOS*>(lParam)->y;

			if (x != _M_x || y != _M_y)
			{
				_M_y = y, _M_x = x;
				MoveWndTo(m_hWnd, x + reinterpret_cast<WINDOWPOS*>(lParam)->cx, y);
			}
			break;
		}

		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

public:
	enum { IDD = IDD_DIALOG1 };

	BOOL SetSubclass(_In_ HWND hwnd)
	{
		if (SubClsWnd::SetSubclass(hwnd, (UINT_PTR)&m_hWnd))
		{
			RECT rc;
			if (GetWindowRect(hwnd, &rc))
			{
				_M_x = rc.left, _M_y = rc.top;
			}
			_M_hwnd = hwnd;
			return TRUE;
		}
		return FALSE;
	}

	BOOL RemoveSubclass()
	{
		if (_M_hwnd)
		{
			if (SubClsWnd::RemoveSubclass(_M_hwnd, (UINT_PTR)&m_hWnd))
			{
				_M_hwnd = 0;
				return TRUE;
			}
			__debugbreak();
		}
		return FALSE;
	}

	QrWnd(PPOINT ppt, HWND hwndWorker) : _M_ppt(ppt), _M_hwndWorker(hwndWorker)
	{
	}
};

class ProvDlg : public CDlgBase
{
	HWND _M_hwndMain = 0;
	HWND _M_hwndQR = 0;
	HWND _M_MyHwnd = 0;
	POINT _M_pt = { CW_USEDEFAULT, CW_USEDEFAULT };

	HWND execute(HWND hwnd)
	{
		if (QrWnd* p = new QrWnd(&_M_pt, _M_MyHwnd))
		{
			if (HWND hwndMy = p->Create(MAKEINTRESOURCEW(QrWnd::IDD), hwnd))
			{
				if (0 <= _M_pt.x && 0 <= _M_pt.y)
				{
					MoveWndTo(hwndMy, _M_pt.x, _M_pt.y);
				}

				p->SetSubclass(hwnd);

				return hwndMy;
			}
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
			SendMessageW(hwndQR, WM_CLOSE, 0, 0);
			SetState(hwndDlg, 0);
		}
	}

	void SetState(HWND hwndDlg, HWND hwndQR)
	{
		_M_hwndQR = hwndQR;
		EnableWindow(GetDlgItem(hwndDlg, IDOK), hwndQR == 0);
		EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), hwndQR != 0);
	}

	void InitPos(HWND hwndDlg)
	{
		RECT rc;
		if (GetWindowRect(hwndDlg, &rc))
		{
			ULONG dx = rc.right - rc.left;
			if (GetWindowRect(_M_hwndMain, &rc))
			{
				MoveWndTo(hwndDlg, rc.left - dx, rc.top);
				_M_pt.x = rc.right;
				_M_pt.y = rc.top;
			}
		}
	}

	virtual INT_PTR DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
			_M_MyHwnd = hwnd;
			InitPos(hwnd);
__ok:
			SetState(hwnd, ShowQr());
			break;

		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;

		case WM_NCDESTROY:
			OnNcDestroy();
__cancel:
			HideQr(hwnd);
			break;

		case WM_APP:
			SetState(hwnd, 0);
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

	ProvDlg(HWND hwndMain) : _M_hwndMain(hwndMain)
	{
	}
};

ULONG WINAPI WorkTp(PVOID hwnd)
{
	ProvDlg dlg((HWND)hwnd);

	dlg.AddRef();

	return (ULONG)dlg.DoModal(MAKEINTRESOURCEW(ProvDlg::IDD));
}

INT_PTR CALLBACK StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case IDCANCEL:
			EndDialog(hwnd, lParam);
			break;
		}
		break;

	case WM_INITDIALOG:
		if (HANDLE hThread = CreateThread(0, 0, WorkTp, hwnd, 0, (ULONG*)&uMsg))
		{
			CloseHandle(hThread);
			SetWindowLongPtrW(hwnd, DWLP_USER, uMsg);
		}
		break;

	case WM_NCDESTROY:
		if (uMsg = (ULONG)GetWindowLongPtrW(hwnd, DWLP_USER))
		{
			PostThreadMessageW(uMsg, WM_QUIT, 0, 0);
		}
		break;
	}

	return 0;
}

void NTAPI ep(void*)
{
	DialogBoxParamW((HINSTANCE)&__ImageBase, MAKEINTRESOURCEW(IDD_DIALOG3), 0, StaticDlgProc, 0);
	MessageBoxW(0, 0, L"Exiting...", MB_ICONINFORMATION);
	ExitProcess(0);
}
