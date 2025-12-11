#pragma once
#include <Windows.h>
#include <cstdint>

#include <Math/Vector/Vector2.h>

class WinApp
{
public: //静的メンバ変数
	// ウィンドウサイズ
	static uint32_t kWindowWidth_;		// 横
	static uint32_t kWindowHeight_;		// 縦

    static Vector2 kWindowSize_;	// サイズ

	static const wchar_t kWindowClassName[];

public: // 静的メンバ関数

	static WinApp* GetInstance();
	static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam);

public: // メンバ関数
    void Initilize(const wchar_t* _title = L"GameEngine",
		uint32_t _clientWidth = kWindowWidth_, uint32_t _clientHeight = kWindowHeight_,
		UINT _style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME);

	void Finalize() const;

	bool ProcessMessage();

	//アクセッサ
	HWND GetHwnd() const { return hwnd_; }
	HINSTANCE GetHInstance()  const { return wndClass_.hInstance; }
private: // メンバ変数
	// ウィンドウクラス
	WNDCLASS wndClass_{};
	// ウィンドウハンドル
	HWND hwnd_ = nullptr;
};
