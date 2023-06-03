#include "stdafx.h"

#include "UiContext.h"

struct WindowCallData 
{
	_In_ window_context* pItf;
	_Out_ PVOID result;
};

EXTERN_C extern IMAGE_DOS_HEADER __ImageBase;

#define MAGIC_WPARAM ((WPARAM)&__ImageBase)

LRESULT CALLBACK window_context::CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// doc error: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644975(v=vs.85)

	if (nCode == HC_ACTION && 
		wParam && // If the message was sent by the current thread, it is zero; otherwise, it is nonzero.
		reinterpret_cast<PCWPSTRUCT>(lParam)->message == WM_NULL &&
		reinterpret_cast<PCWPSTRUCT>(lParam)->wParam == MAGIC_WPARAM) 
	{
		WindowCallData* pCallData = reinterpret_cast<WindowCallData*>(reinterpret_cast<PCWPSTRUCT>(lParam)->lParam);
		pCallData->result = pCallData->pItf->execute(reinterpret_cast<PCWPSTRUCT>(lParam)->hwnd);
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

PVOID window_context::invoke(_In_ HWND hwnd)
{
	ULONG dwProcessId;

	if (ULONG dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId))
	{
		if (dwProcessId == GetCurrentProcessId())
		{
			if (dwThreadId == GetCurrentThreadId())
			{
				return execute(hwnd);
			}

			if (HHOOK hhk = SetWindowsHookExW(WH_CALLWNDPROC, CallWndProc, (HMODULE)&__ImageBase, dwThreadId))
			{
				WindowCallData calldata { this };
				SendMessageW(hwnd, WM_NULL, MAGIC_WPARAM, (LPARAM)&calldata);
				UnhookWindowsHookEx(hhk);
				return calldata.result;
			}
		}
	}

	return 0;
}
