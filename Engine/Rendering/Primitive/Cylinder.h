#pragma once

#include <Rendering/Primitive/Primitive.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>
#include <Physics/Math/quaternion.h>
#include <Rendering/Model/WorldTransform.h>
#include <Rendering/Model/Mesh.h>
#include <Rendering/Model/Material.h>
#include <Rendering/Model/ObjectColor.h>


class Cylinder : public Primitive
{
public:

    Cylinder() = delete;
    Cylinder(float _topRadius, float _bottomRadius, float _height, int32_t _divide = 16, bool _top = true, bool _bottom = true);
    ~Cylinder() = default;
    void Generate() override;
    void Update() override;
    void Draw() override;
    void Draw(const Camera& _camera, const Vector4& _color = { 1,1,1,1 }) override;

    //============ setter ============

    void SetDivide(int32_t _divide) { divide_ = _divide; }
    void SetTopRadius(float _topRadius) { topRadius_ = _topRadius; }
    void SetBottomRadius(float _bottomRadius) { bottomRadius_ = _bottomRadius; }
    void SetHeight(float _height) { height_ = _height; }
    void SetTop(bool _top) { top_ = _top; }
    void SetBottom(bool _bottom) { bottom_ = _bottom; }

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
    int32_t divide_ = 0;

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

    // quaternionを使用するか
    bool useQuaternion_ = false;

    // ワールド変換行列
    WorldTransform worldTransform_ = {};

    // テクスチャハンドル
    uint32_t textureHandle_ = 0;

    // メッシュ
    Mesh mesh_ = {};
    // マテリアル
    Material material_ = {};
    // オブジェクトカラー
    ObjectColor objectColor_ = {};


};