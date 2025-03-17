#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <cstdint>
#include <array>
#include <vector>
#include <numbers>
#include <cmath>
#include <memory>

/**
* @brief 指向性ライト
* @param color		        ライトの色
* @param direction	        ライトの向き
* @param intendity	        輝度
* @param useHalfLambert	    ハーフランバートを使うか
*/
struct DirectionalLight
{
    Matrix4x4 viewProjection = Matrix4x4::Identity();

    Vector4 color = {1,1,1,1};		// ライトの色

    Vector3 direction = { 0,-1,0 };	// ライトの向き
    float intensity = 1.0f;	        // 輝度

    uint32_t isHalf = 1;            // ハーフランバートを使うか
    float pad[3] = {};
};

/**
* @brief 点光源
* @param color		        ライトの色
* @param position_	        ライトの位置
* @param intensity	        輝度
* @param radius	            ライトの影響半径
* @param decay		        減衰率
* @param isHalf	            ハーフランバートを使うか
*/
struct PointLight
{
    Vector4 color = { 1,1,1,1 };	// ライトの色

    Vector3 position = { 0,1,0 };	// ライトの位置
    float intensity = 1.0f;	        // 輝度

    float radius = 5.0f;	        // ライトの影響半径
    float decay = 0.5f;	            // 減衰率
    uint32_t isHalf = 1;            // ハーフランバートを使うか
    float pad = {};
};


/**
* @brief スポットライト
* @param color		        ライトの色
* @param position		    ライトの位置
* @param intensity		    輝度
* @param direction		    ライトの向き
* @param distance		    有効範囲
* @param decay			    減衰率
* @param cosAngle		    角度
* @param falloutStartAngle	開始角度
* @param isHalf			    ハーフランバートを使うか
*/
struct SpotLight
{
    Vector4 color = { 1,1,1,1 };		//ライトの色

    Vector3 position = { 1,1,0 };       //ライトの位置
    float intensity = 4.0f;	            //輝度

    Vector3 direction = { -1,-1,0 };    //ライトの向き
    float distance = 7.0f;              //有効範囲

    float decay = 2.0f;	                // 減衰率
    float cosAngle                      // 角度
        = std::cosf(std::numbers::pi_v<float> / 3.0f);
    float falloutStartAngle             // 開始角度
        = std::cosf(std::numbers::pi_v<float> / 3.0f);
    uint32_t isHalf = 1;
};

/**
* @brief ライトグループ
*/
class LightGroup
{
private:// 定数
    static constexpr size_t MAX_DIRECTIONAL_LIGHT = 1;
    static constexpr size_t MAX_POINT_LIGHT = 32;
    static constexpr size_t MAX_SPOT_LIGHT = 16;


public:
    // 転送用の構造体
    struct LightTransferData
    {
        DirectionalLight directionalLight;

        std::array<PointLight, MAX_POINT_LIGHT> pointLights;

        std::array<SpotLight, MAX_SPOT_LIGHT> spotLights;

        uint32_t numPointLight = 0;
        uint32_t numSpotLight = 0;
        uint32_t pad[2];
    };


public:

    static LightTransferData GetDefaultLightData();
    static void SetShadowMapSize(const Vector2& _size) { shadowMapSize_ = _size; }

    LightGroup() = default;
    ~LightGroup() = default;

    void Initialize();
    void Update();
    void Draw();

    void SetDirectionalLight(const DirectionalLight& _light);
    void AddPointLight(const PointLight& _light, const std::string& _name = "", Vector3* _parent = nullptr);
    void AddSpotLight(const SpotLight& _light, const std::string& _name = "", Vector3* _parent = nullptr);

    void SetEnableDirectionalLight(bool _enable) { enableDirectionalLight_ = _enable; }
    void SetEnablePointLight(bool _enable) { enablePointLight_ = _enable; }
    void SetEnableSpotLight(bool _enable) { enableSpotLight_ = _enable; }

    PointLight& GetPointLight(const std::string& _name);
    SpotLight& GetSpotLight(const std::string& _name);

    void DeletePointLight(const std::string& _name);
    void DeleteSpotLight(const std::string& _name);

    LightTransferData GetLightData();

    void DrawDebugWindow();


private:

    LightTransferData lightData_;

    template<typename T>
    struct NamedLight {
        T light = {};
        std::string name = "";
        bool select = false;
        Vector3* parent = nullptr;
    };

    size_t pointLightCount_ = 0;
    size_t spotLightCount_ = 0;

    std::list<NamedLight<PointLight>> selectablePointLights_;
    std::list<NamedLight<SpotLight>> selectableSpotLights_;

    bool dirty_ = false;    // 更新が必要か

    bool enableDirectionalLight_ = true;
    bool enablePointLight_ = true;
    bool enableSpotLight_ = true;

    DirectionalLight directionalLight_;

    char addPointLightName_[128] = {};
    char addSpotLightName_[128] = {};

    static  Vector2 shadowMapSize_;

    Matrix4x4 LookAt(const Vector3& _eye, const Vector3& _at, const Vector3& _up);
};
