#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>
#include <string>

class Material
{
public:

    /// uvTransform
    Vector2         transform_                      = {0.0f, 0.0f};         // 移動
    Vector2         scale_                          = { 1.0f, 1.0f };       // スケール
    float           rotation_                       = 0.0f;                 // 回転

    float           shiness_                        = 40.0f;                // 

    bool            enableLighting_                 = true;                 // ライティングの有無

    void Initialize(const std::string& _texturepath);

private:

    std::string     name_                           = {};
    std::string     texturePath_                    = {};

    struct Transferdata
    {
        Matrix4x4       uvTransform;
        float           shininess;
        int32_t         enabledLighthig;
        float           padding[2];
    };

    Microsoft::WRL::ComPtr<ID3D12Resource>          resorces_               = nullptr;
    Transferdata*                                   constMap_               = nullptr;;

    void TransferData();

};