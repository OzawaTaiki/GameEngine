#pragma once

#include <Features/Model/Primitive/Primitive.h>


class Ring : public Primitive
{
public:
    Ring(float _innerRadius, float _outerRadius);
    ~Ring() override = default;

    Model* Generate(const std::string& _name) override;
    Primitive* Clone() override { return new Ring(*this); }

    //============ setter ============

    void SetInnerRadius(float _radius) { innerRadius_ = _radius; }
    void SetOuterRadius(float _radius) { outerRadius_ = _radius; }

    void SetStartAngle(float _angle) { startAngle_ = _angle; }
    void SetEndAngle(float _angle) { endAngle_ = _angle; }

    void SetStartOuterRadiusRatio(float _ratio) { startOuterRadiusRatio_ = _ratio; }
    void SetEndOuterRadiusRatio(float _ratio) { endOuterRadiusRatio_ = _ratio; }

    void SetFlipU(bool _flip) { flipU_ = _flip; }
    void SetFlipV(bool _flip) { flipV_ = _flip; }


private:

    void NormalizeAngles();
    void NormalizeRadius();


    float innerRadius_ = -1.0f;
    float outerRadius_ = -1.0f;

    float startAngle_ = 0.0f; // 開始角度
    float endAngle_ = 0.0f; // 終了角度

    float startOuterRadiusRatio_ = 0.0f; // 開始外径比率 内径からの比率(0.0f~1.0f)
    float endOuterRadiusRatio_ = 0.0f; // 終了外径比率 内径からの比率(0.0f~1.0f)


    // texcoordを反転するか
    bool flipU_ = false;
    bool flipV_ = false;

};


