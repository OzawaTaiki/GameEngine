#include "Input.h"

#include <Debug/ImGuiDebugManager.h>

#include <cassert>
#include <algorithm>

Input* Input::GetInstance()
{
    static Input instace;
    return &instace;
}

Input::Input()
{
#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->RegisterMenuItem("InputStatus", [this](bool* _open) { ImGui(_open); });
#endif // _DEBUG

}

void Input::Initilize(WinApp* _winApp)
{
    winApp_ = _winApp;

    HRESULT hresult = S_FALSE;


    hresult = DirectInput8Create(winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
    assert(SUCCEEDED(hresult));

    ///キーボード入力の初期化
    hresult = directInput_->CreateDevice(GUID_SysKeyboard, &keyboardDevice_, NULL);
    assert(SUCCEEDED(hresult));

    hresult = keyboardDevice_->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(hresult));

    hresult = keyboardDevice_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    assert(SUCCEEDED(hresult));


    // マウス入力の初期化
    hresult = directInput_->CreateDevice(GUID_SysMouse, &mouseDevice_, NULL);
    assert(SUCCEEDED(hresult));

    hresult = mouseDevice_->SetDataFormat(&c_dfDIMouse);
    assert(SUCCEEDED(hresult));

    hresult = mouseDevice_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    assert(SUCCEEDED(hresult));

}


void Input::Update()
{

    HRESULT hresult = S_FALSE;
    preKeys_ = keys_;
    //std::memcpy(preKeys_.data(), keys_.data(),keys_.size());

    keyboardDevice_->Acquire();
    hresult= keyboardDevice_->GetDeviceState(DWORD(keys_.size()), keys_.data());

    preMouse_ = mouse_;


    hresult = mouseDevice_->Acquire();
    hresult = mouseDevice_->GetDeviceState(sizeof(DIMOUSESTATE), &mouse_);

    if (enambleVibrate_)
    {
        currentVibrateTime_ += 1.0f / 60.0f;
        if (currentVibrateTime_ >= vibrateTimeMax_)
        {
            StopVibratePad();
        }
    }

    preXInputState_ = xInputState_;
    XInputGetState(0, &xInputState_);
    UpdatePadLRTrigger();


}

bool Input::IsKeyTriggered(uint8_t _key) const
{
    if (keys_[_key] && !preKeys_[_key])
        return true;

    return false;
}

bool Input::IsKeyPressed(uint8_t _key) const
{
    if (keys_[_key] && preKeys_[_key])
        return true;

    return false;
}

bool Input::IsKeyReleased(uint8_t _key) const
{
    if (!keys_[_key] && preKeys_[_key])
        return true;

    return false;
}

bool Input::IsMouseTriggered(uint8_t _buttonNum) const
{
    if (mouse_.rgbButtons[_buttonNum] &&! preMouse_.rgbButtons[_buttonNum])
        return true;
    return false;
}

bool Input::IsMousePressed(uint8_t _buttonNum) const
{
    if (mouse_.rgbButtons[_buttonNum] && preMouse_.rgbButtons[_buttonNum])
        return true;
    return false;
}

bool Input::IsMouseReleased(uint8_t _buttonNum) const
{
    if (!mouse_.rgbButtons[_buttonNum] && preMouse_.rgbButtons[_buttonNum])
        return true;
    return false;
}

float Input::GetMouseWheel() const
{
    if (mouse_.lZ != 0)
    {
        // マウスホイールの回転量を取得
        return static_cast<float>(mouse_.lZ) / WHEEL_DELTA;
    }

    return 0.0f;
}

Vector2 Input::GetMousePosition() const
{
    POINT mousePos;
    GetCursorPos(&mousePos);

    ScreenToClient(winApp_->GetHwnd(), &mousePos);

    Vector2 result;
    result.x = static_cast<float>(mousePos.x);
    result.y = static_cast<float>(mousePos.y);

    return result;
}
void Input::GetMove(Vector3& _move, float _speed) const
{
    _move = { 0,0,0 };
    if (keys_[DIK_W])
        _move.z += 1.0f;
    if (keys_[DIK_A])
        _move.x += -1.0f;
    if (keys_[DIK_S])
        _move.z += -1.0f;
    if (keys_[DIK_D])
        _move.x += 1.0f;


    if (keys_[DIK_SPACE])
        _move.y += 1.0f;
    if (keys_[DIK_LCONTROL])
        _move.y -= 1.0f;

    _move = _move.Normalize() * _speed;
}

void Input::GetRotate(Vector3& _rot, float _sensi) const
{
    _rot.y = mouse_.lX * _sensi;
    _rot.x = mouse_.lY * _sensi;
}
bool Input::IsPadTriggered(PadButton _button) const
{
    if (_button == PadButton::iPad_LT)
    {
        if (leftTrigger_.isTriggered && !leftTrigger_.preIsTriggered)
            return true;
    }
    else if (_button == PadButton::iPad_RT)
    {
        if (rightTrigger_.isTriggered && !rightTrigger_.preIsTriggered)
            return true;
    }
    else
    {
        if (xInputState_.Gamepad.wButtons & static_cast<WORD>(_button) && !(preXInputState_.Gamepad.wButtons & static_cast<WORD>(_button)))
            return true;
    }

    return false;
};

bool Input::IsPadPressed(PadButton _button) const
{
    if (_button == PadButton::iPad_LT)
    {
        if (leftTrigger_.isTriggered)
            return true;
    }
    else if (_button == PadButton::iPad_RT)
    {
        if (rightTrigger_.isTriggered)
            return true;
    }
    else
    {
        if (xInputState_.Gamepad.wButtons & static_cast<WORD>(_button) && preXInputState_.Gamepad.wButtons & static_cast<WORD>(_button))
            return true;
    }

    return false;
}

bool Input::IsPadReleased(PadButton _button) const
{
    if (_button == PadButton::iPad_LT)
    {
        if (!leftTrigger_.isTriggered && leftTrigger_.preIsTriggered)
            return true;
    }
    else if (_button == PadButton::iPad_RT)
    {
        if (!rightTrigger_.isTriggered && rightTrigger_.preIsTriggered)
            return true;
    }
    else
    {
        if (!(xInputState_.Gamepad.wButtons & static_cast<WORD>(_button)) && preXInputState_.Gamepad.wButtons & static_cast<WORD>(_button))
            return true;
    }

    return false;
}

void Input::VibratePad(float _leftMotorSpeed, float _rightMotorSpeed, float _vibrateTime)
{
    enambleVibrate_ = true;
    _leftMotorSpeed = std::clamp(_leftMotorSpeed, 0.0f, 1.0f);
    _rightMotorSpeed = std::clamp(_rightMotorSpeed, 0.0f, 1.0f);

    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = static_cast<WORD>(_leftMotorSpeed * 65535.0f);
    vibration.wRightMotorSpeed = static_cast<WORD>(_rightMotorSpeed * 65535.0f);

    XInputSetState(0, &vibration);

    currentVibrateTime_ = 0.0f;
    vibrateTimeMax_ = _vibrateTime;
}

void Input::StopVibratePad()
{
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = 0;
    vibration.wRightMotorSpeed = 0;

    XInputSetState(0, &vibration);

    enambleVibrate_ = false;
}

Vector2 Input::GetPadLeftStick() const
{
    if (xInputState_.Gamepad.sThumbLX || xInputState_.Gamepad.sThumbLY)
    {
        Vector2 result;
        result.x = static_cast<float>(xInputState_.Gamepad.sThumbLX) / 32767.0f;
        result.y = static_cast<float>(xInputState_.Gamepad.sThumbLY) / 32767.0f;

        if (result.x< deadZone_ && result.x > -deadZone_)
            result.x = 0.0f;
        if (result.y < deadZone_ && result.y > -deadZone_)
            result.y = 0.0f;

        result.x = std::clamp(result.x, -1.0f, 1.0f);
        result.y = std::clamp(result.y, -1.0f, 1.0f);

        return result;
    }
    return Vector2();
}

Vector2 Input::GetPadRightStick() const
{
    if (xInputState_.Gamepad.sThumbRX || xInputState_.Gamepad.sThumbRY)
    {
        Vector2 result;
        result.x = static_cast<float>(xInputState_.Gamepad.sThumbRX) / 32767.0f;
        result.y = static_cast<float>(xInputState_.Gamepad.sThumbRY) / 32767.0f;

        if (result.x < deadZone_ && result.x > -deadZone_)
            result.x = 0.0f;
        if (result.y < deadZone_ && result.y > -deadZone_)
            result.y = 0.0f;

        result.x = std::clamp(result.x, -1.0f, 1.0f);
        result.y = std::clamp(result.y, -1.0f, 1.0f);

        return result;
    }
    return Vector2();
}

void Input::SetStickDeadZone(float _deadZone)
{
    deadZone_ = _deadZone;
}

void Input::SetTriggerDeadZone(float _deadZone)
{
    leftTrigger_.deadZone = _deadZone;
    rightTrigger_.deadZone = _deadZone;
}

bool Input::IsControllerConnected()
{
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));
    // コントローラの状態を取得
    DWORD result = XInputGetState(0, &state);
    // コントローラが接続されている場合は true を返す
    return (result == ERROR_SUCCESS);
}

void Input::UpdatePadLRTrigger()
{
    leftTrigger_.preIsTriggered = leftTrigger_.isTriggered;
    leftTrigger_.isTriggered = false;
    leftTrigger_.value = 0;

    if (xInputState_.Gamepad.bLeftTrigger > leftTrigger_.deadZone)
    {
        leftTrigger_.value = static_cast<float>(xInputState_.Gamepad.bLeftTrigger) / 255.0f;
        leftTrigger_.isTriggered = true;
    }

    rightTrigger_.preIsTriggered = rightTrigger_.isTriggered;
    rightTrigger_.isTriggered = false;
    rightTrigger_.value = 0;

    if (xInputState_.Gamepad.bRightTrigger > rightTrigger_.deadZone)
    {
        rightTrigger_.value = static_cast<float>(xInputState_.Gamepad.bRightTrigger) / 255.0f;
        rightTrigger_.isTriggered = true;
    }
}


#ifdef _DEBUG
void Input::ImGui(bool* _open)
{
    ImGui::Begin("Input", _open);
    {
        ImGui::Text("Mouse left : %s", IsMousePressed(0) ? "Pressed" : "Released");
        ImGui::Text("Mouse right : %s", IsMousePressed(1) ? "Pressed" : "Released");
        ImGui::Text("Mouse middle : %s", IsMousePressed(2) ? "Pressed" : "Released");
        Vector2 mousePos = GetMousePosition();
        ImGui::Text("Mouse Pos : %f %f", mousePos.x, mousePos.y);
        ImGui::Text("Mouse Wheel : %f", GetMouseWheel());

        ImGui::SeparatorText("Controller");
        if (IsControllerConnected())
        {
            ImGui::Text("Left Stick : %f %f", GetPadLeftStick().x, GetPadLeftStick().y);
            ImGui::Text("Right Stick : %f %f", GetPadRightStick().x, GetPadRightStick().y);


            ImGui::SliderFloat("Dead Zone", &deadZone_, 0.0f, 1.0f);

            ImGui::SliderFloat("Left Motor Speed", &leftMotorSpeed_, 0.0f, 1.0f);
            ImGui::SliderFloat("Right Motor Speed", &rightMotorSpeed_, 0.0f, 1.0f);
            ImGui::SliderFloat("Vibrate Time", &vibrateTime_, 0.0f, 10.0f);
            if (ImGui::Button("vibratePad"))
            {
                VibratePad(leftMotorSpeed_, rightMotorSpeed_, vibrateTime_);
            }
            ImGui::SameLine();
            if (ImGui::Button("StopVibratePad"))
            {
                StopVibratePad();
            }
        }
        else
            ImGui::Text("Controller is not connected");

    }



    ImGui::End();
}
#endif // _DEBUG
