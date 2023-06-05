#pragma once

class __declspec(novtable) SubClsWnd
{
	static LRESULT CALLBACK s_MySubclassProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
		)
	{
		return reinterpret_cast<SubClsWnd*>(dwRefData)->MySubclassProc(hWnd, uMsg, wParam, lParam, uIdSubclass);
	}

protected:
	virtual LRESULT CALLBACK MySubclassProc(HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass) = 0;

public:

	BOOL SetSubclass(_In_ HWND hwnd, _In_ UINT_PTR uIdSubclass)
	{
		return SetWindowSubclass(hwnd, s_MySubclassProc, uIdSubclass, (ULONG_PTR)this);
	}

	BOOL RemoveSubclass(_In_ HWND hwnd, _In_ UINT_PTR uIdSubclass)
	{
		return RemoveWindowSubclass(hwnd, s_MySubclassProc, uIdSubclass);
	}
};