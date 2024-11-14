#include "Matrix4x4.h"

#ifdef _DEBUG
#include <imgui.h>
void Matrix4x4::ShowData(const std::string& _label, bool _newWindow) const
{
    if (_newWindow)
        ImGui::Begin("Matrix4x4");

    ImGui::Text(_label.c_str());
    ImGui::Text("%6.3f,%6.3f,%6.3f,%6.3f", m[0][0], m[0][1], m[0][2], m[0][3]);
    ImGui::Text("%6.3f,%6.3f,%6.3f,%6.3f", m[1][0], m[1][1], m[1][2], m[1][3]);
    ImGui::Text("%6.3f,%6.3f,%6.3f,%6.3f", m[2][0], m[2][1], m[2][2], m[2][3]);
    ImGui::Text("%6.3f,%6.3f,%6.3f,%6.3f", m[3][0], m[3][1], m[3][2], m[3][3]);

    if (_newWindow)
        ImGui::End();
}
#endif // _DEBUG
