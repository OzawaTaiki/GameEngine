#include "Ring.h"

#include <Math/MyLib.h>
#include <Features/Model/Manager/ModelManager.h>

#include <algorithm>
#include <numbers>

Ring::Ring(float _innerRadius, float _outerRadius) :
    innerRadius_(_innerRadius),
    outerRadius_(_outerRadius),
    startAngle_(0.0f),
    endAngle_(0.0f),
    startOuterRadiusRatio_(1.0f),
    endOuterRadiusRatio_(1.0f),
    flipU_(false),
    flipV_(false)
{
}

Model* Ring::Generate(const std::string& _name)
{
    NormalizeAngles();
    NormalizeRadius();

    // 内半径と外半径の比率を計算
    float startOuterRadius = Lerp(innerRadius_, outerRadius_, startOuterRadiusRatio_);
    float endOuterRadius = Lerp(innerRadius_, outerRadius_, endOuterRadiusRatio_);

    // 円環の頂点数を計算
    int32_t vertexCount = (divide_) * 2;
    int32_t indexCount = divide_ * 6;
    std::vector<VertexData> vertices(vertexCount);
    std::vector<uint32_t> indices(indexCount);

    const float kRadianPerDivide = (endAngle_ - startAngle_) / divide_;

    // 頂点の座標を計算
    for (int32_t index = 0; index < vertexCount; index += 2)
    {
        float angle = startAngle_ + kRadianPerDivide * (index / 2.0f);

        float cos = std::cos(angle);
        float sin = std::sin(angle);


        // 外側の頂点
        vertices[index].position.x = cos * outerRadius_;
        vertices[index].position.y = sin * outerRadius_;
        vertices[index].position.z = 0.0f;
        vertices[index].position.w = 1.0f;

        vertices[index].normal = { 0.0f,1.0f,0.0f };

        vertices[index].texcoord = { static_cast<float>(index) / static_cast<float>(divide_),0.0f };

        // 内側の頂点
        vertices[index + 1].position.x = cos * innerRadius_;
        vertices[index + 1].position.y = sin * innerRadius_;
        vertices[index + 1].position.z = 0.0f;
        vertices[index + 1].position.w = 1.0f;

        vertices[index + 1].normal = { 0.0f,1.0f,0.0f };

        vertices[index + 1].texcoord = { static_cast<float>(index + 1) / static_cast<float>(divide_),1.0f };


        if (flipU_)
        {
            vertices[index].texcoord.x = 1.0f - vertices[index].texcoord.x;
            vertices[index + 1].texcoord.x = 1.0f - vertices[index + 1].texcoord.x;
        }
        if (flipV_)
        {
            vertices[index].texcoord.y = 1.0f - vertices[index].texcoord.y;
            vertices[index + 1].texcoord.y = 1.0f - vertices[index + 1].texcoord.y;
        }
    }

    for (uint32_t index = 0; index < divide_; index++)
    {
        // インデックスの設定
        indices[index * 6 + 0] = index * 2;
        indices[index * 6 + 1] = index * 2 + 1;
        indices[index * 6 + 2] = (index * 2 + 2) % (vertexCount);

        indices[index * 6 + 3] = (index * 2 + 2) % (vertexCount);
        indices[index * 6 + 4] = index * 2 + 1;
        indices[index * 6 + 5] = (index * 2 + 3) % (vertexCount);
    }

    // TODO: 開始終了半径を考慮した頂点座標の補完
    // スプラインで外径を補完する


    Model* model = Model::CreateFromVertices(vertices, indices, _name);

    return model;
}


void Ring::NormalizeAngles()
{
    // 開始角度が終了角度より大きい場合はスワップ
    if (startAngle_ > endAngle_) {
        std::swap(startAngle_, endAngle_);
        std::swap(startOuterRadiusRatio_, endOuterRadiusRatio_);
    }

    float PI = std::numbers::pi_v<float>;

    // 角度が完全な円（2π）より大きい場合、2πで割った余りに正規化
    startAngle_ = std::fmod(startAngle_, 2.0f * PI);
    if (startAngle_ < 0) startAngle_ += 2.0f * PI;

    endAngle_ = std::fmod(endAngle_, 2.0f * PI);
    if (endAngle_ < 0) endAngle_ += 2.0f * PI;

    // 開始角度と終了角度が同じ場合、完全な円にする
    if (std::abs(startAngle_ - endAngle_) < 0.0001f) {
        endAngle_ = startAngle_ + 2.0f * PI;
    }
    // 終了角度が開始角度より小さい場合、2π加える
    else if (endAngle_ < startAngle_) {
        endAngle_ += 2.0f * PI;
    }

    startOuterRadiusRatio_ = std::clamp(startOuterRadiusRatio_, 0.0f, 1.0f);
    endOuterRadiusRatio_ = std::clamp(endOuterRadiusRatio_, 0.0f, 1.0f);
}

void Ring::NormalizeRadius()
{
    // 内半径と外半径の大小を比較し、必要に応じてスワップ
    if (innerRadius_ > outerRadius_)
    {
        std::swap(innerRadius_, outerRadius_);
    }
    // 内半径が0未満の場合、0に設定
    if (innerRadius_ < 0.0f)
    {
        innerRadius_ = 0.0f;
    }
    // 外半径が内半径より小さい場合、内半径を外半径に設定
    if (outerRadius_ < innerRadius_)
    {
        outerRadius_ = innerRadius_;
    }
}
