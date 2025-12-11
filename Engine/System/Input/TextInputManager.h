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
    std::wstring GetInputWText();

    void Clear();

    /// <summary>
    /// DirectInputのキーコードを文字に変換
    /// </summary>
    /// <param name="keyCode">DirectInputキーコード (DIK_*)</param>
    /// <param name="shiftPressed">Shiftキーが押されているか</param>
    /// <param name="outChar">出力される文字</param>
    /// <returns>変換成功時true、変換不可の場合false</returns>
    bool ConvertDIKeyToChar(uint8_t keyCode, bool shiftPressed, wchar_t& outChar);

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