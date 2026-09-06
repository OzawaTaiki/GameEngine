#pragma once
#include <Windows.h>
#include <cstdint>

#include <Core/WinApp/Screen.h>
#include <Math/Vector/Vector2.h>


namespace Engine {

class WinApp
{
public: //静的メンバ変数
	// 設定が無いときのウィンドウサイズ
	static constexpr uint32_t kDefaultWidth_ = 1280;
	static constexpr uint32_t kDefaultHeight_ = 720;

	static const wchar_t kWindowClassName[];

public: // 静的メンバ関数

	static WinApp* GetInstance();
	static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam);

public: // メンバ関数
	// 実際のウィンドウサイズは Screen::Width() / Screen::Height() から取得する
    void Initilize(const wchar_t* _title = L"GameEngine",
		uint32_t _clientWidth = kDefaultWidth_, uint32_t _clientHeight = kDefaultHeight_,
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

} // namespace Engine
