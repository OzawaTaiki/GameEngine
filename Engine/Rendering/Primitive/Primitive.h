#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>

#include <vector>
#include <cstdint>


class Primitive
{
public:

    Primitive() = default;
    virtual ~Primitive() = default;

    virtual void Generate() = 0;

protected:
    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 uv;
    };

    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
};

