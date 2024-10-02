#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp
{
public:
	// ウィンドウサイズ
	static const uint32_t kWindowWidth_ = 1280;		// 横
	static const uint32_t kWindowHeight_ = 720;		// 縦

	static const wchar_t kWindowClassName[];

	static WinApp* GetInstance();

	static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam);

	void CreateGameWindow(const wchar_t* _title = L"XGame", 
						  UINT _style = WS_OVERLAPPEDWINDOW, 
						  uint32_t _clientWidth = kWindowWidth_, uint32_t _clientHeight = kWindowHeight_);

	HWND GetHwnd() { return hwnd_; }
	HINSTANCE GetHInstance() { return wndClass_.hInstance; }
private:
	// ウィンドウクラス
	WNDCLASS wndClass_{};
	// ウィンドウハンドル
	HWND hwnd_ = nullptr;
};