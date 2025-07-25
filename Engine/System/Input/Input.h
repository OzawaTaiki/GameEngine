#pragma once
#include <Core/WinApp/WinApp.h>

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <xinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"XInput.lib")

#include <d3d12.h>
#include <wrl.h>
#include <array>


enum class PadButton
{
	iPad_Up		= XINPUT_GAMEPAD_DPAD_UP,			// ↑
	iPad_Down	= XINPUT_GAMEPAD_DPAD_DOWN,			// ↓
    iPad_Left	= XINPUT_GAMEPAD_DPAD_LEFT,			// ←
    iPad_Right	= XINPUT_GAMEPAD_DPAD_RIGHT,		// →
    iPad_Start	= XINPUT_GAMEPAD_START,				// Start
    iPad_Back	= XINPUT_GAMEPAD_BACK,				// Back
    iPad_LS		= XINPUT_GAMEPAD_LEFT_THUMB,		// LS
    iPad_RS		= XINPUT_GAMEPAD_RIGHT_THUMB,		// RS
    iPad_LB     = XINPUT_GAMEPAD_LEFT_SHOULDER,		// LB
    iPad_RB     = XINPUT_GAMEPAD_RIGHT_SHOULDER,	// RB
    iPad_LT     = 0x0400,                           // LT
    iPad_RT     = 0x0800,                           // RT
    iPad_A		= XINPUT_GAMEPAD_A,					// A
    iPad_B		= XINPUT_GAMEPAD_B,					// B
    iPad_X		= XINPUT_GAMEPAD_X,					// X
    iPad_Y		= XINPUT_GAMEPAD_Y,					// Y



	iPad_Max       // ボタン数
};

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

    float GetMouseWheel() const;

	Vector2 GetMousePosition() const;
    void GetMove(Vector3& _move, float _spped = 1.0f) const;
    void GetRotate(Vector3& _rot, float _sensi = 0.001f) const;

	bool IsPadTriggered(PadButton _button) const;
    bool IsPadPressed(PadButton _button) const;
    bool IsPadReleased(PadButton _button) const;

    // パッドの振動
    // _leftMotorSpeed : 左モーターのスピード(0.0f ~ 1.0f)
    // _rightMotorSpeed : 右モーターのスピード(0.0f ~ 1.0f)
    // _vibrateTime : 振動時間(秒)
    void VibratePad(float _leftMotorSpeed, float _rightMotorSpeed, float _vibrateTime);

    void StopVibratePad();

    // パッドの左スティックの値を取得
    // 戻り値 : パッドの左スティックの値
    Vector2 GetPadLeftStick() const;
    // パッドの右スティックの値を取得
    // 戻り値 : パッドの右スティックの値
    Vector2 GetPadRightStick() const;

    /// <summary>
    /// スティックのデッドゾーンを設定
    /// </summary>
    /// <param name="_deadZone">: 0 ~ 1 </param>
    void SetStickDeadZone(float _deadZone);

    /// <summary>
    /// LT RT のデッドゾーンを設定
    /// </summary>
    /// <param name="_deadZone">: 0 ~ 1 </param>
    void SetTriggerDeadZone(float _deadZone);

    /// <summary>
    /// コントローラーが接続されているかどうかを確認
    /// </summary>
    /// <returns></returns>
    bool IsControllerConnected();

private:
	Microsoft::WRL::ComPtr <IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr <IDirectInputDevice8> keyboardDevice_ = nullptr;
	Microsoft::WRL::ComPtr <IDirectInputDevice8> mouseDevice_ = nullptr;

	std::array<BYTE, 256> keys_ = {};
	std::array<BYTE, 256> preKeys_ = {};

	DIMOUSESTATE mouse_ = {};
	DIMOUSESTATE preMouse_ = {};

    struct PadLRTrgger
    {
        bool isTriggered = false;
        bool preIsTriggered = false;
        float value = 0.0f;
        float deadZone = 0.1f;
    };

    // LRTriggerのデータ
    void UpdatePadLRTrigger();

    _XINPUT_STATE xInputState_ = {};
    _XINPUT_STATE preXInputState_ = {};
    PadLRTrgger leftTrigger_;
    PadLRTrgger rightTrigger_;
    float currentVibrateTime_ = 0.0f;
    float vibrateTimeMax_ = 0.0f;
    float deadZone_ = 0.1f;
    bool enambleVibrate_ = false;


	WinApp* winApp_ = nullptr;

private:
    // コピーコンストラクタ
    Input();
    ~Input() = default;
    //代入演算子を削除
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;


#ifdef _DEBUG
    float leftMotorSpeed_ = 0.0f;
    float rightMotorSpeed_ = 0.0f;
    float vibrateTime_ = 0.0f;

    void ImGui(bool* _open);
#endif // _DEBUG

};
