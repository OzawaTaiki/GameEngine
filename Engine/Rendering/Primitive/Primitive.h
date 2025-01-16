#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>

#include <Rendering/Model/Mesh.h>

#include <vector>
#include <cstdint>


class Primitive
{
public:

    Primitive() = default;
    virtual ~Primitive() = default;

    virtual void Generate() = 0;

protected:

    std::vector<Mesh::VertexData> vertices_;
    std::vector<uint32_t> indices_;
};

