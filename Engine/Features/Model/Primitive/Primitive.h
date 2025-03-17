#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>


#include <Features/Model/Mesh/Mesh.h>

#include <vector>
#include <cstdint>


class Camera;

class Primitive
{
public:

    Primitive() = default;
    virtual ~Primitive() = default;

    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Draw(const Camera& _camera, const Vector4& _color = { 1,1,1,1 }) = 0;

    virtual void Generate() = 0;

protected:

    std::vector<VertexData> vertices_;
    std::vector<uint32_t> indices_;
};

