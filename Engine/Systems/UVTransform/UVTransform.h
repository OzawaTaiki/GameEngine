#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Matrix4x4.h>

class UVTransform
{
public:

    UVTransform();
    ~UVTransform() = default;


    // Setters
    void SetUOffset(float offset) { uOffset_ = offset; }
    void SetVOffset(float offset) { vOffset_ = offset; }
    void SetOffset(float uOffset, float vOffset) { uOffset_ = uOffset; vOffset_ = vOffset; }
    void SetOffset(const Vector2& offset) { uOffset_ = offset.x; vOffset_ = offset.y; }
    void SetUScale(float scale) { uScale_ = scale; }
    void SetVScale(float scale) { vScale_ = scale; }
    void SetScale(float uScale, float vScale) { uScale_ = uScale; vScale_ = vScale; }
    void SetScale(const Vector2& scale) { uScale_ = scale.x; vScale_ = scale.y; }
    void SetRotation(float rotation) { rotation_ = rotation; }

    // Getters
    float GetUOffset() const { return uOffset_; }
    float GetVOffset() const { return vOffset_; }
    Vector2 GetOffset() const { return Vector2(uOffset_, vOffset_); }
    float GetUScale() const { return uScale_; }
    float GetVScale() const { return vScale_; }
    Vector2 GetScale() const { return Vector2(uScale_, vScale_); }
    float GetRotation() const { return rotation_; }


    Matrix4x4 GetMatrix() const;

private:

    float uOffset_ = 0.0f;
    float vOffset_ = 0.0f;
    float uScale_ = 1.0f;
    float vScale_ = 1.0f;
    float rotation_ = 0.0f;




};
