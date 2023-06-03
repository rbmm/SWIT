#include "stdafx.h"

#include "wnd.h"

LRESULT SimpWndClsNoRefImpl::wDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	++_M_dwMessageCount;

	lParam = DialogProc(hwndDlg, uMsg, wParam, lParam);

	if (!--_M_dwMessageCount)
	{
		AfterLastMessage();
		Release();
	}

	return lParam;
}

LRESULT CALLBACK SimpWndClsNoRefImpl::__DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		SetWindowLongPtr(hwndDlg, DWLP_DLGPROC, (LONG_PTR)_DialogProc);
		reinterpret_cast<SimpWndClsNoRefImpl*>(lParam)->AddRef();
		reinterpret_cast<SimpWndClsNoRefImpl*>(lParam)->_M_dwMessageCount = 1 << 31;
		return reinterpret_cast<SimpWndClsNoRefImpl*>(lParam)->wDialogProc(hwndDlg, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT SimpWndClsNoRefImpl::DialogProc(HWND /*hwndDlg*/, UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	switch (uMsg)
	{
	case WM_NCDESTROY:
		_bittestandreset(&_M_dwMessageCount, 31);
		break;
	}
	return 0;
}