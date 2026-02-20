#pragma once

#include <Features/Model/Primitive/Primitive.h>


namespace Engine {

class Triangle : public Primitive
{
public:
    Triangle() = default;
    ~Triangle() override = default;

    Model* Generate(const std::string& _name) override;
    Primitive* Clone() override { return new Triangle(*this); }

    // 上 右 左
    void SetVertices(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2);
    // 上 右 左
    void SetVertices(const std::array<Vector3, 3>& _vertices);

    void SetVertex0(const Vector3& _v0) { vertices_[0] = _v0; }
    void SetVertex1(const Vector3& _v1) { vertices_[1] = _v1; }
    void SetVertex2(const Vector3& _v2) { vertices_[2] = _v2; }

    void SetNormal(const Vector3& _normal) { normal_ = _normal; }

    void SetFlipU(bool _flipU) { flipU_ = _flipU; }
    void SetFlipV(bool _flipV) { flipV_ = _flipV; }


private:

    void CalculateVertices();


    void CalculateNormal();

private:

    std::array<Vector3, 3> vertices_ = {
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(-1.0f, -1.0f, 0.0f),
        Vector3(1.0f, -1.0f, 0.0f)
    };

    std::array<Vector2, 3> texcoords_ = {
        Vector2(0.5f, 0.0f),
        Vector2(1.0f, 1.0f),
        Vector2(0.0f, 1.0f),
    };

    Vector3 normal_ = Vector3(0.0f, 0.0f, -1.0f);

    bool flipU_ = false;
    bool flipV_ = false;



};

} // namespace Engine
