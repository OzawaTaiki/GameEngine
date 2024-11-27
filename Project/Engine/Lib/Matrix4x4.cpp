#include "Matrix4x4.h"


Vector3 Matrix4x4::GetScale() const
{
    Vector3 scale;

    scale.x = Vector3(m[0][0], m[0][1], m[0][2]).Length();
    scale.y = Vector3(m[1][0], m[1][1], m[1][2]).Length();
    scale.z = Vector3(m[2][0], m[2][1], m[2][2]).Length();

    return scale;
}

Vector3 Matrix4x4::GetTranslate() const
{
    Vector3 translate;

    translate.x = m[3][0];
    translate.y = m[3][1];
    translate.z = m[3][2];

    return translate;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& _mat)
{
    Matrix4x4 result;
    Matrix4x4 _m1 = *this;
    Matrix4x4 _m2 = _mat;

    result.m[0][0] = _m1.m[0][0] * _m2.m[0][0] + _m1.m[0][1] * _m2.m[1][0] + _m1.m[0][2] * _m2.m[2][0] + _m1.m[0][3] * _m2.m[3][0];
    result.m[0][1] = _m1.m[0][0] * _m2.m[0][1] + _m1.m[0][1] * _m2.m[1][1] + _m1.m[0][2] * _m2.m[2][1] + _m1.m[0][3] * _m2.m[3][1];
    result.m[0][2] = _m1.m[0][0] * _m2.m[0][2] + _m1.m[0][1] * _m2.m[1][2] + _m1.m[0][2] * _m2.m[2][2] + _m1.m[0][3] * _m2.m[3][2];
    result.m[0][3] = _m1.m[0][0] * _m2.m[0][3] + _m1.m[0][1] * _m2.m[1][3] + _m1.m[0][2] * _m2.m[2][3] + _m1.m[0][3] * _m2.m[3][3];

    result.m[1][0] = _m1.m[1][0] * _m2.m[0][0] + _m1.m[1][1] * _m2.m[1][0] + _m1.m[1][2] * _m2.m[2][0] + _m1.m[1][3] * _m2.m[3][0];
    result.m[1][1] = _m1.m[1][0] * _m2.m[0][1] + _m1.m[1][1] * _m2.m[1][1] + _m1.m[1][2] * _m2.m[2][1] + _m1.m[1][3] * _m2.m[3][1];
    result.m[1][2] = _m1.m[1][0] * _m2.m[0][2] + _m1.m[1][1] * _m2.m[1][2] + _m1.m[1][2] * _m2.m[2][2] + _m1.m[1][3] * _m2.m[3][2];
    result.m[1][3] = _m1.m[1][0] * _m2.m[0][3] + _m1.m[1][1] * _m2.m[1][3] + _m1.m[1][2] * _m2.m[2][3] + _m1.m[1][3] * _m2.m[3][3];

    result.m[2][0] = _m1.m[2][0] * _m2.m[0][0] + _m1.m[2][1] * _m2.m[1][0] + _m1.m[2][2] * _m2.m[2][0] + _m1.m[2][3] * _m2.m[3][0];
    result.m[2][1] = _m1.m[2][0] * _m2.m[0][1] + _m1.m[2][1] * _m2.m[1][1] + _m1.m[2][2] * _m2.m[2][1] + _m1.m[2][3] * _m2.m[3][1];
    result.m[2][2] = _m1.m[2][0] * _m2.m[0][2] + _m1.m[2][1] * _m2.m[1][2] + _m1.m[2][2] * _m2.m[2][2] + _m1.m[2][3] * _m2.m[3][2];
    result.m[2][3] = _m1.m[2][0] * _m2.m[0][3] + _m1.m[2][1] * _m2.m[1][3] + _m1.m[2][2] * _m2.m[2][3] + _m1.m[2][3] * _m2.m[3][3];

    result.m[3][0] = _m1.m[3][0] * _m2.m[0][0] + _m1.m[3][1] * _m2.m[1][0] + _m1.m[3][2] * _m2.m[2][0] + _m1.m[3][3] * _m2.m[3][0];
    result.m[3][1] = _m1.m[3][0] * _m2.m[0][1] + _m1.m[3][1] * _m2.m[1][1] + _m1.m[3][2] * _m2.m[2][1] + _m1.m[3][3] * _m2.m[3][1];
    result.m[3][2] = _m1.m[3][0] * _m2.m[0][2] + _m1.m[3][1] * _m2.m[1][2] + _m1.m[3][2] * _m2.m[2][2] + _m1.m[3][3] * _m2.m[3][2];
    result.m[3][3] = _m1.m[3][0] * _m2.m[0][3] + _m1.m[3][1] * _m2.m[1][3] + _m1.m[3][2] * _m2.m[2][3] + _m1.m[3][3] * _m2.m[3][3];

    return result;
}

Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& _mat)
{
    *this = *this * _mat;
    return *this;
}

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
