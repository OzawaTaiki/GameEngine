#include "Cube.h"


namespace Engine {

namespace
{
std::array<Vector3, 8> defaultVertices_ = {
    // 手前
    Vector3{-0.5f, -0.5f, -0.5f}, // 0: LB
    Vector3{-0.5f,  0.5f, -0.5f}, // 1: LT
    Vector3{ 0.5f, -0.5f, -0.5f}, // 2: RB
    Vector3{ 0.5f,  0.5f, -0.5f}, // 3: RT
    // 奥
    Vector3{-0.5f, -0.5f,  0.5f}, // 4: LB
    Vector3{-0.5f,  0.5f,  0.5f}, // 5: LT
    Vector3{ 0.5f, -0.5f,  0.5f}, // 6: RB
    Vector3{ 0.5f,  0.5f,  0.5f}, // 7: RT
};
}

Model* Cube::Generate(const std::string& _name)
{
    auto vertices = CalculateVertices();
    std::vector<VertexData> vertexData;
    std::vector<uint32_t> indices;

    auto AddFace = [&](const std::array<int, 4>& idx, const Vector3& normal)
        {
            uint32_t base = static_cast<uint32_t>(vertexData.size());
            vertexData.push_back({ Vector4(vertices[idx[0]], 1.0f), {0.0f, 1.0f}, normal });
            vertexData.push_back({ Vector4(vertices[idx[1]], 1.0f), {0.0f, 0.0f}, normal });
            vertexData.push_back({ Vector4(vertices[idx[2]], 1.0f), {1.0f, 1.0f}, normal });
            vertexData.push_back({ Vector4(vertices[idx[3]], 1.0f), {1.0f, 0.0f}, normal });

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 2);
            indices.push_back(base + 1);
            indices.push_back(base + 3);
        };

    // 前面 (-Z)
    AddFace({ 0, 1, 2, 3 }, { 0.0f, 0.0f, -1.0f });
    // 背面 (+Z)
    AddFace({ 5, 4, 7, 6 }, { 0.0f, 0.0f, 1.0f });
    // 左面 (-X)
    AddFace({ 4, 5, 0, 1 }, { -1.0f, 0.0f, 0.0f });
    // 右面 (+X)
    AddFace({ 2, 3, 6, 7 }, { 1.0f, 0.0f, 0.0f });

    // 上面 (+Y)
    if (hasTop_)
    {
        AddFace({ 1, 5, 3, 7 }, { 0.0f, 1.0f, 0.0f });
    }
    // 底面 (-Y)
    if (hasBottom_)
    {
        AddFace({ 4, 0, 6, 2 }, { 0.0f, -1.0f, 0.0f });
    }

    // === モデル生成 ===
    Model* model = Model::CreateFromVertices(vertexData, indices, _name);
    return model;
}

std::array<Vector3, 8> Cube::CalculateVertices() const
{
    std::array<Vector3, 8> vertices = defaultVertices_;


    for (auto& v : vertices)
    {
        v.x = v.x - pivot_.x * 0.5f;
        v.y = v.y - pivot_.y * 0.5f;
        v.z = v.z - pivot_.z * 0.5f;

        v.x *= size_.x;
        v.y *= size_.y;
        v.z *= size_.z;
    }

    return vertices;
}

} // namespace Engine
