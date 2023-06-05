#include "stdafx.h"

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// doc error: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644975(v=vs.85)

	if (nCode == HC_ACTION && 
		wParam && // If the message was sent by the current thread, it is zero; otherwise, it is nonzero.
		reinterpret_cast<PCWPSTRUCT>(lParam)->message == WM_NULL) 
	{
		void** ppv = (void**)reinterpret_cast<PCWPSTRUCT>(lParam)->lParam;
		
		*ppv = reinterpret_cast<PVOID (WINAPI*)(HWND , PVOID)>(reinterpret_cast<PCWPSTRUCT>(lParam)->wParam)(
			reinterpret_cast<PCWPSTRUCT>(lParam)->hwnd,	*ppv);
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

PVOID invoke_in_ui(_In_ HWND hwnd, PVOID (WINAPI *cb)(HWND hwnd, PVOID Parameter), _In_ PVOID Parameter)
{
	ULONG dwProcessId;

	if (ULONG dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId))
	{
		if (dwProcessId == GetCurrentProcessId())
		{
			if (dwThreadId == GetCurrentThreadId())
			{
				return cb(hwnd, Parameter);
			}

			if (HHOOK hhk = SetWindowsHookExW(WH_CALLWNDPROC, CallWndProc, (HMODULE)&__ImageBase, dwThreadId))
			{
				SendMessageW(hwnd, WM_NULL, (WPARAM)cb, (LPARAM)&Parameter);
				UnhookWindowsHookEx(hhk);
				return Parameter;
			}
		}
	}

	return 0;
}
