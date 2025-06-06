#include "Plane.h"

#include <Math/Quaternion/Quaternion.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>

// z+向き板 いらんかも //TODO
std::array<Vector3, 4> Plane::defaultVertices_ = {
   Vector3( 1.0f,  1.0f ,0.0f),
   Vector3(-1.0f,  1.0f ,0.0f),
   Vector3( 1.0f, -1.0f ,0.0f),
   Vector3(-1.0f, -1.0f ,0.0f)
};

Model* Plane::Generate(const std::string& _name)
{
    std::vector<VertexData> vertices(4);
    std::vector<uint32_t> indices(6);

    std::array<Vector3, 4> vertex = CalculateVertices();

    //UVの計算

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        vertices[i].position = vertex[i];
        vertices[i].position.w = 1.0f;
        vertices[i].normal = normal_;
        vertices[i].texcoord = uvs_[i];

        if (flipU_)
        {
            vertices[i].texcoord.x = 1.0f - vertices[i].texcoord.x;
        }
        if (flipV_)
        {
            vertices[i].texcoord.y = 1.0f - vertices[i].texcoord.y;
        }

    }


    indices[0] = 2;
    indices[1] = 1;
    indices[2] = 0;

    indices[3] = 3;
    indices[4] = 1;
    indices[5] = 2;

    Model* model = Model::CreateFromVertices(vertices, indices, _name);

    return model;
}


std::array<Vector3, 4> Plane::CalculateVertices()
{
    std::array<Vector3, 4> vertices = defaultVertices_;
    normal_ = normal_.Normalize();
    const Vector3 defaultNormal = { 0.0f, 0.0f, -1.0f };
    Quaternion q = Quaternion::FromToRotation(defaultNormal, normal_);
    Matrix4x4 rotationMatrix = q.Normalize().ToMatrix();

    Vector4 halfSize = size_ * 0.5f;

    for (uint32_t i = 0; i < vertices.size(); ++i)
    {
        // 基準点を適用
        vertices[i] -= pivot_;

        // サイズを適用
        vertices[i].x *= halfSize.x;
        vertices[i].y *= halfSize.y;
        vertices[i].z *= halfSize.z;

    }

    for (uint32_t i = 0; i < vertices.size(); ++i)
    {
        // 回転を適用
        vertices[i] = Transform(vertices[i], rotationMatrix);
    }

    return vertices;
}