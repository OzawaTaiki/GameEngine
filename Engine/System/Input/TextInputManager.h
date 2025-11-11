#pragma once

#include <queue>
#include <string>


class TextInputManager
{
public:
    static TextInputManager* GetInstance();

    void PushChar(wchar_t ch);

    void SetAcceptingInput(bool accepting) { isAcceptingInput_ = accepting; }

    std::string GetInputText();

    void Clear();

private:

    std::wstring inputBuffer_;

    // 入力受付フラグ
    bool isAcceptingInput_ = true;

private:// シングルトンパターン
    TextInputManager() = default;
    ~TextInputManager() = default;
    TextInputManager(const TextInputManager&) = delete;
    TextInputManager& operator=(const TextInputManager&) = delete;

};