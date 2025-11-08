#pragma once

#include <Features/Model/Model.h>

#include <cstdint>
#include <string>
#include <algorithm>

struct UVRange
{
    float uMin = 0.0f;
    float vMin = 0.0f;
    float uMax = 1.0f;
    float vMax = 1.0f;
};


class Primitive
{
public:

    virtual ~Primitive() = default;

    virtual Model* Generate(const std::string& _name) = 0;
    virtual Primitive* Clone() = 0;

    void SetDivide(uint32_t _divide) { divide_ = (std::max)(3u, _divide); }
    void SetUVRange(const UVRange& _uvRange) { uvRange_ = _uvRange; }
    void SetUVRange(float _uMin, float _vMin, float _uMax, float _vMax)
    {
        uvRange_.uMin = std::max(0.0f, _uMin);
        uvRange_.vMin = std::max(0.0f, _vMin);
        uvRange_.uMax = std::min(1.0f, _uMax);
        uvRange_.vMax = std::min(1.0f, _vMax);
    }

    const std::string& GetName() const { return name_; }

protected:
    uint32_t divide_ = 16;
    std::string name_ = "";
    UVRange uvRange_;
};

