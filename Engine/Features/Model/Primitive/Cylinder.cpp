#include <Features/Model/Primitive/Cylinder.h>

#include <Features/LineDrawer/LineDrawer.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Light/System/LightingSystem.h>



namespace Engine {

Cylinder::Cylinder(float _topRadius, float _bottomRadius, float _height) :
    topRadius_(_topRadius),
    bottomRadius_(_bottomRadius),
    height_(_height),
    top_(false),
    bottom_(false),
    startAngle_(0.0f),
    endAngle_(0.0f),
    flipU_(false),
    flipV_(false)
{
}

Model* Cylinder::Generate(const std::string& _name)
{
    NormalizeAngles();
    NormalizeRadius();

    // 円環の頂点数を計算
    int32_t vertexCount = (divide_) * 2;
    int32_t indexCount = divide_ * 6;
    if (!loop_)
        indexCount -= 6;
    std::vector<VertexData> vertices(vertexCount);
    std::vector<uint32_t> indices(indexCount);

    const float kRadianPerDivide = (endAngle_ - startAngle_) / divide_;

    // 頂点の座標を計算
    for (int32_t index = 0; index < vertexCount; index += 2)
    {
        float angle = startAngle_ + kRadianPerDivide * (index / 2.0f);

        float cos = std::cos(angle);
        float sin = std::sin(angle);


        // 上側の頂点
        vertices[index].position.x = cos * topRadius_;
        vertices[index].position.y = height_;
        vertices[index].position.z = sin * topRadius_;
        vertices[index].position.w = 1.0f;

        //vertices[index].normal = { 0.0f,1.0f,0.0f };

        vertices[index].texcoord = { static_cast<float>(index) / static_cast<float>(divide_),0.0f };

        // 下側の頂点
        vertices[index + 1].position.x = cos * bottomRadius_;
        vertices[index + 1].position.y = 0.0f;
        vertices[index + 1].position.z = sin * bottomRadius_;
        vertices[index + 1].position.w = 1.0f;

        //vertices[index + 1].normal = { 0.0f,1.0f,0.0f };

        vertices[index + 1].texcoord = { static_cast<float>(index + 1) / static_cast<float>(divide_),1.0f };


        Vector3 normal = { cos,0.0f,sin };
        vertices[index].normal = normal;
        vertices[index + 1].normal = normal;


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

    for (uint32_t index = 0; index < indexCount / 6u; index++)
    {
        // インデックスの設定
        indices[index * 6 + 0] = index * 2;
        indices[index * 6 + 1] = (index * 2 + 2) % (vertexCount);
        indices[index * 6 + 2] = index * 2 + 1;

        indices[index * 6 + 3] = index * 2 + 1;
        indices[index * 6 + 4] = (index * 2 + 2) % (vertexCount);
        indices[index * 6 + 5] = (index * 2 + 3) % (vertexCount);
    }


    Model* model = Model::CreateFromVertices(vertices, indices, _name);

    return model;

}

void Cylinder::NormalizeAngles()
{
    // 開始角度が終了角度より大きい場合はスワップ
    if (startAngle_ > endAngle_) {
        std::swap(startAngle_, endAngle_);
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
        loop_ = true;
    }
    // 終了角度が開始角度より小さい場合、2π加える
    else if (endAngle_ < startAngle_) {
        endAngle_ += 2.0f * PI;
    }

}

void Cylinder::NormalizeRadius()
{
    // 上下の半径が負の場合、0にする
    if (topRadius_ < 0.0f) {
        topRadius_ = 0.0f;
    }
    if (bottomRadius_ < 0.0f) {
        bottomRadius_ = 0.0f;
    }
}

} // namespace Engine
