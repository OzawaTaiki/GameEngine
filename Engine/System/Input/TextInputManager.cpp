#include "TextInputManager.h"
#include <Utility/ConvertString/ConvertString.h>

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
