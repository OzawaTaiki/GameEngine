#include "Input.h"

#include <cassert>

Input* Input::GetInstance()
{
    static Input instace;
    return &instace;
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
