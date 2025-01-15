#include "Ring.h"
#include <Rendering/LineDrawer/LineDrawer.h>

#include <numbers>

Ring::Ring(float _innerRadius, float _outerRadius, uint32_t _divide, std::array<bool, 3> _billboard) :
    innerRadius_(_innerRadius),
    outerRadius_(_outerRadius),
    divide_(_divide),
    billboard_(_billboard)
{
}

void Ring::Generate()
{
    vertices_.clear();
    indices_.clear();

    // 頂点ごとの角度
    float kDivideAngle = std::numbers::pi_v<float> *2.0f / divide_;
    // 一頂点ごとのUV座標の割合
    float kDivideUV = 1.0f / divide_;

    // 頂点の計算　頂点の数 ＝ 分割数
    for (uint32_t index = 0; index < divide_; ++index)
    {
        // 外側の頂点
        Vertex outerVertex;
        // 内側の頂点
        Vertex innerVertex;

        float angle = kDivideAngle * static_cast<float>(index);

        outerVertex.position = Vector3(std::cosf(angle) * outerRadius_, std::sinf(angle) * outerRadius_, 0.0f);
        outerVertex.normal = Vector3(0.0f, 0.0f, -1.0f);
        outerVertex.uv = { kDivideUV * static_cast<float>(index),1.0f };


        innerVertex.position = Vector3(std::cosf(angle) * innerRadius_, std::sinf(angle) * innerRadius_, 0.0f);
        innerVertex.normal = Vector3(0.0f, 0.0f, -1.0f);
        innerVertex.uv = { kDivideUV * static_cast<float>(index),0.0f };

        vertices_.push_back(outerVertex);
        vertices_.push_back(innerVertex);
    }

    // インデックスの計算
    for (uint32_t index = 0; index < divide_ * 2; index += 2)
    {
        //  021 123 243 345 465...
        indices_.push_back((index + 2) % (divide_ * 2));
        indices_.push_back((index + 0) % (divide_ * 2));
        indices_.push_back((index + 1) % (divide_ * 2));

        indices_.push_back((index + 1) % (divide_ * 2));
        indices_.push_back((index + 2) % (divide_ * 2));
        indices_.push_back((index + 3) % (divide_ * 2));
    }


}

void Ring::Draw()
{
    static uint32_t count= 1;

    for (uint32_t index = 0; index < indices_.size(); index += 3)
    {
        uint32_t index0 = indices_[index + 0];
        uint32_t index1 = indices_[index + 1];
        uint32_t index2 = indices_[index + 2];

        LineDrawer::GetInstance()->RegisterPoint(vertices_[index0].position, vertices_[index1].position);
        LineDrawer::GetInstance()->RegisterPoint(vertices_[index1].position, vertices_[index2].position);
        LineDrawer::GetInstance()->RegisterPoint(vertices_[index2].position, vertices_[index0].position);

    }

    count += 3;

}
