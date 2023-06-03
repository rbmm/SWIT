#pragma once

class __declspec(novtable) SimpWndClsNoRefImpl 
{
	LONG _M_dwMessageCount;

	static LRESULT CALLBACK __DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static INT_PTR CALLBACK _DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return reinterpret_cast<SimpWndClsNoRefImpl*>(GetWindowLongPtrW(hwndDlg, DWLP_USER))->wDialogProc(hwndDlg, uMsg, wParam, lParam);
	}

	LRESULT wDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	virtual ~SimpWndClsNoRefImpl() = default;

	virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void AfterLastMessage()
	{
	}

public:
	virtual ULONG STDMETHODCALLTYPE AddRef() = 0;

	virtual ULONG STDMETHODCALLTYPE Release() = 0;

	HWND Create(_In_ PCWSTR lpTemplateName, _In_opt_ HWND hWndParent = 0, _In_opt_ HINSTANCE hInstance = (HINSTANCE)&__ImageBase)
	{
		return CreateDialogParamW(hInstance, lpTemplateName, hWndParent, __DialogProc, (LPARAM)this);
	}
	
	INT_PTR Modal(_In_ PCWSTR lpTemplateName, _In_opt_ HWND hWndParent = 0, _In_opt_ HINSTANCE hInstance = (HINSTANCE)&__ImageBase)
	{
		return DialogBoxParamW(hInstance, lpTemplateName, hWndParent, __DialogProc, (LPARAM)this);
	}
};

class __declspec(novtable) SimpWndCls : public SimpWndClsNoRefImpl
{
	LONG _M_dwRef = 1;

public:

	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return InterlockedIncrementNoFence(&_M_dwRef);
	}

	virtual ULONG STDMETHODCALLTYPE Release()
	{
		if (ULONG dwRef = InterlockedDecrement(&_M_dwRef)) 
		{
			return dwRef;
		}
		delete this;
		return 0;
	}
};

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