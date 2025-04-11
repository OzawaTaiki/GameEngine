#pragma once

#include <Features/Model/Primitive/Primitive.h>

class Cylinder : public Primitive
{
public:

    Cylinder(float _topRadius, float _bottomRadius, float _height, int32_t _divide = 16, bool _top = true, bool _bottom = true);
    ~Cylinder() = default;

    Model* Generate(const std::string& _name) override { return nullptr; };
    Primitive* Clone() override { return new Cylinder(*this); }


    //============ setter ============

    void SetDivide(int32_t _divide) { divide_ = _divide; }
    void SetTopRadius(float _topRadius) { topRadius_ = _topRadius; }
    void SetBottomRadius(float _bottomRadius) { bottomRadius_ = _bottomRadius; }
    void SetHeight(float _height) { height_ = _height; }
    void SetTop(bool _top) { top_ = _top; }
    void SetBottom(bool _bottom) { bottom_ = _bottom; }

    

private:

    // 上半径
    float topRadius_ = 0;
    // 下半径
    float bottomRadius_ = 0;
    // 高さ
    float height_ = 0;

    // 上面の有無
    bool top_ = false;
    // 下面の有無
    bool bottom_ = false;



};
