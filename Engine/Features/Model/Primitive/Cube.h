#pragma once

#include <Features/Model/Primitive/Primitive.h>


namespace Engine {

class Cube : public Primitive
{
public:
    Cube() = default;
    ~Cube() override = default;

    Model* Generate(const std::string& _name) override;
    Primitive* Clone() override { return new Cube(*this); }

    // ============ setter ============
    // 各辺のサイズ設定
    void SetSize(const Vector3& _size) { size_ = _size; }
    // 基準点を設定 (-1~1)
    void SetPivot(const Vector3& _pivot) { pivot_ = _pivot; }
    // texcoord反転設定
    void SetFlipU(bool _flip) { flipU_ = _flip; }
    // texcoord反転設定
    void SetFlipV(bool _flip) { flipV_ = _flip; }
    // 上面の有無
    void HasTop(bool _top) { hasTop_ = _top; }
    // 下面の有無
    void HasBottom(bool _bottom) { hasBottom_ = _bottom; }

    // 分割数はないので削除
    void SetDivide(uint32_t _divide) = delete;
private:

    std::array<Vector3, 8> CalculateVertices()const;

private:

    // 各辺のサイズ設定
    Vector3 size_ ={ 1.0f, 1.0f , 1.0f };

    // 基準点を設定 (-1~1)
    Vector3 pivot_ ={ 0.0f, 0.0f, 0.0f };

    bool flipU_ = false;
    bool flipV_ = false;

    bool hasTop_ = true;
    bool hasBottom_ = true;

};

} // namespace Engine
