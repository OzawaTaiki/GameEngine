#include <Core/WinApp/WinApp.h>
#include <cassert>

#ifdef _DEBUG
#include <imgui_impl_win32.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

#pragma comment (lib,"winmm.lib")

const wchar_t WinApp::kWindowClassName[] = L"WindowClass";

uint32_t WinApp::kWindowWidth_ = 1280;
uint32_t WinApp::kWindowHeight_ = 720;
Vector2 WinApp::kWindowSize_ = Vector2{ static_cast<float>(kWindowWidth_), static_cast<float>(kWindowHeight_) };

WinApp* WinApp::GetInstance()
{
    static WinApp instance;
    return &instance;
}

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef _DEBUG
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return true;
	}

#endif // _DEBUG

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

void WinApp::Initilize(const wchar_t* _title, uint32_t _width, uint32_t _height, UINT _style)
{
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	//ウィンドウプロシージャ
	wndClass_.lpfnWndProc = WindowProc;
	// ウィンドウクラス名
	wndClass_.lpszClassName = kWindowClassName;
	// インスタンスハンドル
	wndClass_.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wndClass_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wndClass_);

    kWindowWidth_ = _width;
    kWindowHeight_ = _height;

    kWindowSize_ = Vector2{ static_cast<float>(_width), static_cast<float>(_height) };

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

	timeBeginPeriod(1);
}

void WinApp::Finalize() const
{
	CloseWindow(hwnd_);
	CoUninitialize();
}

bool WinApp::ProcessMessage()
{
	MSG msg{};

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT)
		return true;

	return false;
}
