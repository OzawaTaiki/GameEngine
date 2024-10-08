#include "Input.h"

#include "WinApp.h"

#include <cassert>

Input* Input::GetInstanse()
{
    static Input instace;
    return &instace;
}

void Input::Initilize()
{
    HRESULT hresult = S_FALSE;

    hresult = DirectInput8Create(WinApp::GetInstance()->GetHInstance() , DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
    assert(SUCCEEDED(hresult));

    hresult = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
    assert(SUCCEEDED(hresult));

    hresult = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(hresult));

    hresult = keyboard_->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(hresult));

}

void Input::Update()
{
    std::copy(keys_.begin(), keys_.end(), preKeys_.begin());

    keyboard_->Acquire();
    keyboard_->GetDeviceState(keys_.size(), keys_.data());
}

bool Input::IsTriger(uint8_t _key)
{
    if (keys_[_key] && !preKeys_[_key])
        return true;

    return false;
}

bool Input::IsPush(uint8_t _key)
{
    if (keys_[_key] && preKeys_[_key])
        return true;

    return false;
}

bool Input::IsRelease(uint8_t _key)
{
    if (!keys_[_key] && preKeys_[_key])
        return true;

    return false;
}
