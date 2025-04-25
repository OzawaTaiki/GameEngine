#pragma once

#include <Features/Model/Primitive/Primitive.h>

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>

#include <array>

class Plane : public Primitive
{
public:
    Plane() = default;
    ~Plane() override = default;

    Model* Generate(const std::string& _name) override;
    Primitive* Clone() override { return new Plane(*this); }

    void SetNormal(const Vector3& _normal) { normal_ = _normal; }

    void SetSize(const Vector2& _size) { size_ = _size; }
    void SetSize(const Vector4& _size) { size_ = _size; }

    void SetFlipU(bool _flip) { flipU_ = _flip; }
    void SetFlipV(bool _flip) { flipV_ = _flip; }

private:

    std::array<Vector3,4> CalculateVertices();


private:

    static std::array<Vector3, 4> defaultVertices_;

    std::array<Vector2, 4> uvs_ = {
        Vector2(0.0f, 0.0f),
        Vector2(1.0f, 0.0f),
        Vector2(0.0f, 1.0f),
        Vector2(1.0f, 1.0f)
    };

    Vector3 normal_ = { 0.0f,0.0f,1.0f };

    // 上 左 右 下
    Vector4 size_ = { 1.0f, 1.0f ,1.0f, 1.0f };


    bool flipU_ = false;
    bool flipV_ = false;



};