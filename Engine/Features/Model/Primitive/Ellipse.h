#pragma once

#include <Features/Model/Primitive/Primitive.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>

#include <Features/Model/Transform/WorldTransform.h>
#include <Features/Model/Mesh/Mesh.h>
#include <Features/Model/Material/Material.h>
#include <Features/Model/Color/ObjectColor.h>

class Camera;

// namespace Primitive
class EllipseModel : public Primitive
{
public:
    EllipseModel() = delete;
    EllipseModel(const Vector3& _radius, uint32_t _divide = 12);
    EllipseModel(float _radius, uint32_t _divide = 12);

    ~EllipseModel() = default;

    void Generate() override;

    void Update() override;

    void Draw() override;
    void Draw(const Camera& _camera, const Vector4& _color = { 1,1,1,1 }) override;


    //============ setter ============

    void SetRadius(float _radius) { radius_ = { _radius,_radius,_radius }; }
    void SetRadius(const Vector2& _radius) { radius_ = _radius; }
    void SetDivide(uint32_t _divide) { divide_ = _divide; }
    void SetUseQuaternion(bool _useQuaternion) { useQuaternion_ = _useQuaternion; }
    void SetTexture(const std::string& _path);
    void SetTexture(uint32_t _handle) { textureHandle_ = _handle; }

    //=================================

    // スケール
    Vector3 scale_ = {};
    // 回転
    Vector3 rotation_ = {};
    // 回転
    Quaternion quternion_ = {};
    // 位置
    Vector3 translate_ = {};

private:

    // x,y半径
    Vector3 radius_ = {};
    // 分割数
    int32_t divide_ = 0;

    // quaternionを使用するか
    bool useQuaternion_ = false;
    // ワールド変換行列
    WorldTransform worldTransform_ = {};

    uint32_t textureHandle_ = 0;

    // メッシュ
    Mesh mesh_ = {};
    // マテリアル
    Material material_ = {};
    // オブジェクトカラー
    ObjectColor objectColor_ = {};

};

