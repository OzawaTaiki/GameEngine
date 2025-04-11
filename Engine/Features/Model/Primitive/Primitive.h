#pragma once

#include <Features/Model/Model.h>

#include <cstdint>
#include <string>



class Primitive
{
public:

    virtual ~Primitive() = default;

    virtual Model* Generate(const std::string& _name) = 0;
    virtual Primitive* Clone() = 0;

    void SetDivide(uint32_t _divide) { divide_ = (std::max)(3u, _divide); }

    const std::string& GetName() const { return name_; }

protected:
    uint32_t divide_ = 16;
    std::string name_ = "";
};

