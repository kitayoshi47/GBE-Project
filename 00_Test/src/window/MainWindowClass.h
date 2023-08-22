#pragma once

#include <windows.h>
#include <wrl.h>

class MainWindowClass {
public:
    MainWindowClass();
    ~MainWindowClass();

    void    Initialize(HINSTANCE hInstance, WNDPROC proc, LPCWSTR name, int width, int height);
    HWND    GetHandle();
    LPCWSTR GetTitle();

private:
    HWND    hWnd;
    LPCWSTR title;
};
