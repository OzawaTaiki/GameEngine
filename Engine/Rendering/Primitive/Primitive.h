#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>


#include <Rendering/Model/Mesh.h>

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

    std::vector<Mesh::VertexData> vertices_;
    std::vector<uint32_t> indices_;
};

