#pragma once

#include <Rendering/Primitive/Primitive.h>

#include <array>

class Ring : public Primitive
{
public:
    Ring(float _innerRadius, float _outerRadius, uint32_t _divide = 16, std::array<bool, 3> _billboard = { true,true,true });
    ~Ring() = default;

    void Generate() override;

    void Draw();

private:

    // 内側の半径
    float innerRadius_ = 0.0f;
    // 外側の半径
    float outerRadius_ = 0.0f;
    // 円周の分割数
    uint32_t divide_ = 0;

    std::array<bool, 3> billboard_ = false;



};

