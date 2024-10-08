#include "WinApp.h"

#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

const wchar_t WinApp::kWindowClassName[] = L"WindowClass";

WinApp* WinApp::GetInstance()
{
    static WinApp instance;
    return &instance;
}

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return true;
	}

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg)
	{
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して，アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::CreateGameWindow(const wchar_t* _title, UINT _style, uint32_t _width, uint32_t _height)
{
	//ウィンドウプロシージャ
	wndClass_.lpfnWndProc = WindowProc;
	// ウィンドウクラス名
	wndClass_.lpszClassName = kWindowClassName;
	// インスタンスハンドル
	wndClass_.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wndClass_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wndClass_);

	RECT wrc = { 0,0,(LONG)_width,(LONG)_height };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(
		wndClass_.lpszClassName,
		_title,
		_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wndClass_.hInstance,
		nullptr
	);

	ShowWindow(hwnd_, SW_NORMAL);

}
