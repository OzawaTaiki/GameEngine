#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp
{
public: //静的メンバ変数
	// ウィンドウサイズ
	static const uint32_t kWindowWidth_ = 1280;		// 横
	static const uint32_t kWindowHeight_ = 720;		// 縦

	static const wchar_t kWindowClassName[];

public: // 静的メンバ関数

	static WinApp* GetInstance();
	static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam);

public: // メンバ関数
	void Initilize(const wchar_t* _title = L"XGame",
				   UINT _style = WS_OVERLAPPEDWINDOW,
				   uint32_t _clientWidth = kWindowWidth_, uint32_t _clientHeight = kWindowHeight_);
	void Filalze();

	bool ProcessMessage();

	//アクセッサ
	HWND GetHwnd() { return hwnd_; }
	HINSTANCE GetHInstance() { return wndClass_.hInstance; }
private: // メンバ変数
	// ウィンドウクラス
	WNDCLASS wndClass_{};
	// ウィンドウハンドル
	HWND hwnd_ = nullptr;
};