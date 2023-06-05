#include "stdafx.h"

#include "UiContext.h"

typedef PVOID (WINAPI *UI_CB)(HWND hwnd, PVOID Parameter);

struct WindowCallData 
{
	_In_ UI_CB cb;
	_In_ PVOID Parameter;
	_Out_ PVOID result;
};

EXTERN_C extern IMAGE_DOS_HEADER __ImageBase;

#define MAGIC_WPARAM ((WPARAM)&__ImageBase)

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// doc error: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644975(v=vs.85)

	if (nCode == HC_ACTION && 
		wParam && // If the message was sent by the current thread, it is zero; otherwise, it is nonzero.
		reinterpret_cast<PCWPSTRUCT>(lParam)->message == WM_NULL &&
		reinterpret_cast<PCWPSTRUCT>(lParam)->wParam == MAGIC_WPARAM) 
	{
		WindowCallData* pCallData = reinterpret_cast<WindowCallData*>(reinterpret_cast<PCWPSTRUCT>(lParam)->lParam);
		pCallData->result = pCallData->cb(reinterpret_cast<PCWPSTRUCT>(lParam)->hwnd, pCallData->Parameter);
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

PVOID invoke_in_ui(_In_ HWND hwnd, UI_CB cb, _In_ PVOID Parameter)
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
				WindowCallData wc { cb, Parameter };
				SendMessageW(hwnd, WM_NULL, MAGIC_WPARAM, (LPARAM)&wc);
				UnhookWindowsHookEx(hhk);
				return wc.result;
			}
		}
	}

	return 0;
}
