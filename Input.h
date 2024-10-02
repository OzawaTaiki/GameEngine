#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include <d3d12.h>
#include <wrl.h>
#include <array>

class Input
{
public:

	static Input* GetInstanse();

	void Initilize();
	void Update();

	bool IsTriger(uint8_t _key);
	bool IsPush(uint8_t _key);
	bool IsRelease(uint8_t _key);

private:
	Microsoft::WRL::ComPtr <IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr <IDirectInputDevice8> keyboard_= nullptr;

	std::array<BYTE, 256> keys_ = {};
	std::array<BYTE, 256> preKeys_ = {};


};