#pragma once

typedef PVOID (WINAPI *UI_CB)(HWND hwnd, PVOID Parameter);

PVOID invoke_in_ui(_In_ HWND hwnd, UI_CB cb, _In_ PVOID Parameter);