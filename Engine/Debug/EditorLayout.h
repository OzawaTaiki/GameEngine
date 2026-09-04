#pragma once

#include <cstdint>


namespace Engine {

/// <summary>
/// エディタ風のデフォルトドックレイアウトを構築する
/// imgui.ini が存在しない初回起動時のみ自動で組み，
/// 以降は ImGui が保存したレイアウトを尊重する
/// </summary>
class EditorLayout
{
public:

    static EditorLayout* GetInstance();

    /// <summary>
    /// 初期化
    /// imgui.ini の有無を見るため，ImGui::CreateContext() の直後に呼ぶこと
    /// </summary>
    void Initialize();

    /// <summary>
    /// 必要ならレイアウトを構築する
    /// DockSpaceOverViewport() を呼ぶ直前に，前フレームで得たIDを渡す
    /// </summary>
    /// <param name="_dockSpaceID">ドックスペースのID (0 のときは何もしない)</param>
    void BuildIfNeeded(uint32_t _dockSpaceID);

    /// <summary> 次のフレームでデフォルトレイアウトに戻す </summary>
    void RequestReset() { needBuild_ = true; }

private:

    void Build(uint32_t _dockSpaceID);

    bool needBuild_ = false;

private:
    EditorLayout() = default;
    ~EditorLayout() = default;
    // コピー禁止
    EditorLayout(const EditorLayout&) = delete;
    EditorLayout& operator=(const EditorLayout&) = delete;

};

} // namespace Engine
