#pragma once

#include <Features/Model/Primitive/Primitive.h>

class Cylinder : public Primitive
{
public:

    Cylinder(float _topRadius, float _bottomRadius, float _height);
    ~Cylinder() = default;

    Model* Generate(const std::string& _name) override;
    Primitive* Clone() override { return new Cylinder(*this); }


    //============ setter ============

    void SetTopRadius(float _topRadius) { topRadius_ = _topRadius; }

    void SetBottomRadius(float _bottomRadius) { bottomRadius_ = _bottomRadius; }
    void SetHeight(float _height) { height_ = _height; }

    void SetTop(bool _top) { top_ = _top; }
    void SetBottom(bool _bottom) { bottom_ = _bottom; }

    void SetStartAngle(float _angle) { startAngle_ = _angle; }
    void SetEndAngle(float _angle) { endAngle_ = _angle; }

    void SetFlipU(bool _flip) { flipU_ = _flip; }
    void SetFlipV(bool _flip) { flipV_ = _flip; }
    

private:
    void NormalizeAngles();
    void NormalizeRadius();

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

    float startAngle_ = 0.0f; // 開始角度
    float endAngle_ = 0.0f; // 終了角度

    // texcoordを反転するか
    bool flipU_ = false;
    bool flipV_ = false;



};
