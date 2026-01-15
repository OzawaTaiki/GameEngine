#include "Triangle.h"


#include <Math/Quaternion/Quaternion.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>


namespace Engine {

Model* Triangle::Generate(const std::string& _name)
{
    CalculateNormal();
    CalculateVertices();

    std::vector<VertexData> vertices(3);
    std::vector<uint32_t> indices(3);

    for (uint32_t index = 0; index < indices.size(); ++index)
    {

        vertices[index].position = vertices_[index];
        vertices[index].position.w = 1.0f;

        vertices[index].normal = normal_;

        vertices[index].texcoord = texcoords_[index];

        indices[index] = index;
    }


    if (flipU_)
    {
        vertices[0].texcoord.x = 1.0f - vertices[0].texcoord.x;
        vertices[1].texcoord.x = 1.0f - vertices[1].texcoord.x;
        vertices[2].texcoord.x = 1.0f - vertices[2].texcoord.x;
    }
    if (flipV_)
    {
        vertices[0].texcoord.y = 1.0f - vertices[0].texcoord.y;
        vertices[1].texcoord.y = 1.0f - vertices[1].texcoord.y;
        vertices[2].texcoord.y = 1.0f - vertices[2].texcoord.y;
    }

    Model* model = Model::CreateFromVertices(vertices, indices, _name);

    return model;


}

void Triangle::SetVertices(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2)
{
    vertices_[0] = _v0;
    vertices_[1] = _v1;
    vertices_[2] = _v2;
}

void Triangle::SetVertices(const std::array<Vector3, 3>& _vertices)
{
    SetVertices(_vertices[0], _vertices[1], _vertices[2]);
}

void Triangle::CalculateVertices()
{
    Vector3 v0 = vertices_[1] - vertices_[0];
    Vector3 v1 = vertices_[2] - vertices_[0];

    Vector3 normal = v0.Cross(v1);

    Quaternion q = Quaternion::FromToRotation(normal, normal_);
    Matrix4x4 rotationMatrix = q.Normalize().ToMatrix();

    for (size_t i = 0; i < vertices_.size(); ++i)
    {
        vertices_[i] = Transform(vertices_[i], rotationMatrix);
    }

}

void Triangle::CalculateNormal()
{
    normal_ = normal_.Normalize();
}

} // namespace Engine
