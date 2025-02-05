#pragma once

#include <Features/Model/Primitive/Primitive.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <Features/Model/Transform/WorldTransform.h>
#include <Features/Model/Mesh/Mesh.h>
#include <Features/Model/Material/Material.h>
#include <Features/Model/Color/ObjectColor.h>
#include <array>

class Camera;

class Ring : public Primitive
{
public:
    Ring();
    Ring(float _innerRadius, float _outerRadius, uint32_t _divide = 16, std::array<bool, 3> _billboard = { true,true,true });
    ~Ring() = default;

    void Generate() override;

    void Update() override;

    void Draw() override;
    void Draw(const Camera& _camera, const Vector4& _color = { 1,1,1,1 }) override;


    //============ setter ============

    void SetInnerRadius(float _radius) { innerRadius_ = _radius; }
    void SetOuterRadius(float _radius) { outerRadius_ = _radius; }
    void SetDivide(uint32_t _divide) { divide_ = _divide; }
    void SetBillboard(std::array<bool, 3> _billboard) { billboard_ = _billboard; }
    void SetUseQuaternion(bool _useQuaternion) { useQuaternion_ = _useQuaternion; }
    void SetTexture(const std::string& _path);
    void SetTexture(const uint32_t _handle) { textureHandle_ = _handle; }
    void SetParent(const WorldTransform* _parent) { worldTransform.parent_ = _parent; }

    //================================

    // スケール
    Vector3 scale_ = {};
    // 回転
    Vector3 rotation_ = {};
    // 回転
    Quaternion quternion_ = {};
    // 位置
    Vector3 translate_ = {};

private:

    // 内側の半径
    float innerRadius_ = 0.0f;
    // 外側の半径
    float outerRadius_ = 0.0f;
    // 円周の分割数
    int32_t divide_ = 0;
    // ビルボード有効フラグ
    std::array<bool, 3> billboard_ = {};

    // クォータニオンを使用するか
    bool useQuaternion_ = true;
    // ワールド変換行列
    WorldTransform worldTransform = {};

    uint32_t textureHandle_ = 0;
    // メッシュ
    Mesh mesh_ = {};
    // マテリアル
    Material material_ = {};
    // オブジェクトカラー
    ObjectColor objectColor_ = {};


};

