#include "TextInputManager.h"
#include <Utility/ConvertString/ConvertString.h>
#include <Windows.h>

TextInputManager* TextInputManager::GetInstance()
{
    static TextInputManager instance;
    return &instance;
}

void TextInputManager::PushChar(wchar_t ch)
{
    if (!isAcceptingInput_)
    {
        return;
    }

    inputBuffer_.push_back(ch);
}

std::string TextInputManager::GetInputText()
{
    std::string result = ConvertString(inputBuffer_);
    inputBuffer_.clear();
    return result;
}

std::wstring TextInputManager::GetInputWText()
{
    std::wstring result = inputBuffer_;
    inputBuffer_.clear();
    return result;
}

void TextInputManager::Clear()
{
    inputBuffer_.clear();
}


bool TextInputManager::ConvertDIKeyToChar(uint8_t dikCode, bool shift, wchar_t& outChar)
{
    // DIK から仮想キーコード(VK)に変換
    UINT vkCode = MapVirtualKey(dikCode, MAPVK_VSC_TO_VK_EX);
    if (vkCode == 0) return false;

    // キーボード状態を設定
    BYTE keyState[256] = {};
    if (shift)
    {
        keyState[VK_SHIFT] = 0x80;
    }

    // Unicode文字に変換
    wchar_t buffer[2] = {};
    int result = ToUnicodeEx(
        vkCode,
        dikCode,
        keyState,
        buffer,
        2,
        0,
        GetKeyboardLayout(0)
    );

    if (result == 1)
    {
        outChar = buffer[0];
        return true;
    }

    return false;
}
