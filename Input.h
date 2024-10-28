#pragma once

#include "Vector2.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include <d3d12.h>
#include <wrl.h>
#include <array>

#include "WinApp.h"

class Input
{
public:

	static Input* GetInstance();

	void Initilize(WinApp* _winApp);
	void Update();

	bool IsKeyTriggered(uint8_t _key) const;
	bool IsKeyPressed(uint8_t _key) const;
	bool IsKeyReleased(uint8_t _key) const;

	bool IsMouseTriggered(uint8_t _buttonNum) const;
	bool IsMousePressed(uint8_t _buttonNum) const;
	bool IsMouseReleased(uint8_t _buttonNum) const;

	Vector2 GetMousePosition() const;

private:
	Microsoft::WRL::ComPtr <IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr <IDirectInputDevice8> keyboardDevice_ = nullptr;
	Microsoft::WRL::ComPtr <IDirectInputDevice8> mouseDevice_ = nullptr;

	std::array<BYTE, 256> keys_ = {};
	std::array<BYTE, 256> preKeys_ = {};

	DIMOUSESTATE mouse_ = {};
	DIMOUSESTATE preMouse_ = {};


	WinApp* winApp_ = nullptr;

};