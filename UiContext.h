#pragma once

class __declspec(novtable) window_context
{
	static LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
protected:
	virtual PVOID execute(_In_ HWND hwnd) = 0;
public:
	PVOID invoke(_In_ HWND hwnd);
};
