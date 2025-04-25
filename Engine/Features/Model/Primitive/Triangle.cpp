#include "Triangle.h"

Model* Triangle::Generate(const std::string& _name)
{
    CalculateNormal();

    std::vector<VertexData> vertices(3);
    std::vector<uint32_t> indices(3);

    vertices[0].position = vertices_[0];
    vertices[0].normal = normal_;
    vertices[0].texcoord = { 0.0f,0.0f };

    vertices[1].position = vertices_[1];
    vertices[1].normal = normal_;
    vertices[1].texcoord = { 0.0f,1.0f };

    vertices[2].position = vertices_[2];
    vertices[2].normal = normal_;
    vertices[2].texcoord = { 1.0f,1.0f };

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
    if (autoNormal_)
    {
        CalculateNormal();
    }
}

void Triangle::SetVertices(const std::array<Vector3, 3>& _vertices)
{
    SetVertices(_vertices[0], _vertices[1], _vertices[2]);
}

void Triangle::CalculateNormal()
{
    if (autoNormal_)
    {
        Vector3 v0 = vertices_[1] - vertices_[0];
        Vector3 v1 = vertices_[2] - vertices_[0];

        normal_ = v0.Cross(v1);
    }
    normal_ = normal_.Normalize();
}