#include <Debug/EditorLayout.h>

#ifdef _DEBUG

#include <imgui.h>
#include <imgui_internal.h>

#include <filesystem>

#endif // _DEBUG


namespace Engine {

EditorLayout* EditorLayout::GetInstance()
{
    static EditorLayout instance;
    return &instance;
}

void EditorLayout::Initialize()
{
#ifdef _DEBUG

    const char* iniFilename = ImGui::GetIO().IniFilename;

    // 保存されたレイアウトが無い(初回起動)ときだけ組む
    needBuild_ = (iniFilename == nullptr) || !std::filesystem::exists(iniFilename);

#endif // _DEBUG
}

void EditorLayout::BuildIfNeeded([[maybe_unused]] uint32_t _dockSpaceID)
{
#ifdef _DEBUG

    // ドックスペースのIDは DockSpaceOverViewport() を一度呼ばないと分からない
    if (_dockSpaceID == 0 || !needBuild_)
        return;

    Build(_dockSpaceID);
    needBuild_ = false;

#endif // _DEBUG
}

void EditorLayout::Build([[maybe_unused]] uint32_t _dockSpaceID)
{
#ifdef _DEBUG

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGuiID dockMainID = static_cast<ImGuiID>(_dockSpaceID);

    // ImGuiDockNodeFlags_DockSpace は internal 側の別enumなのでそのままORできない
    const ImGuiDockNodeFlags nodeFlags =
        static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_DockSpace) |
        static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::DockBuilderRemoveNode(dockMainID);
    ImGui::DockBuilderAddNode(dockMainID, nodeFlags);
    ImGui::DockBuilderSetNodeSize(dockMainID, viewport->WorkSize);

    // 中央(dockMainID)を残しながら周囲を切り出していく
    const ImGuiID dockLeftID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, 0.19f, nullptr, &dockMainID);
    const ImGuiID dockRightID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, 0.28f, nullptr, &dockMainID);
    const ImGuiID dockDownID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, 0.30f, nullptr, &dockMainID);

    // 左 : 一覧
    ImGui::DockBuilderDockWindow("Hierarchy", dockLeftID);
    // 右 : 選択中のものの中身
    ImGui::DockBuilderDockWindow("Inspector", dockRightID);
    // 下 : ログと計測値。同じノードに入れるとタブになる
    ImGui::DockBuilderDockWindow("Console", dockDownID);
    ImGui::DockBuilderDockWindow("Stats", dockDownID);
    // 中央 : ゲーム画面
    ImGui::DockBuilderDockWindow("Game", dockMainID);

    ImGui::DockBuilderFinish(static_cast<ImGuiID>(_dockSpaceID));

#endif // _DEBUG
}

} // namespace Engine
