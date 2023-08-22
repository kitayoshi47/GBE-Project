#include "MainWindowClass.h"

MainWindowClass::MainWindowClass()
    : hWnd(nullptr)
    , title(L"")
{
}

MainWindowClass::~MainWindowClass()
{
}

void MainWindowClass::Initialize(HINSTANCE hInstance, WNDPROC proc, LPCWSTR name, int width, int height)
{
    // ウィンドウクラスを初期化
    WNDCLASSEX windowClass = {};
    windowClass.cbSize        = sizeof(WNDCLASSEX);
    windowClass.style         = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc   = proc;
    windowClass.hInstance     = hInstance;
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = name;
    RegisterClassEx(&windowClass);

    // ウィンドウのサイズを求める
    RECT windowRect = { 0, 0, width, height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // ウィンドウハンドルを作成
    hWnd = CreateWindow(
        name,
        name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    title = name;
}

HWND MainWindowClass::GetHandle()
{
    return hWnd;
}

LPCWSTR MainWindowClass::GetTitle()
{
    return title;
}
