// ポイントライトシャドウマップ生成コンピュートシェーダー

// 入力データ構造体
struct VertexData
{
    float4 position;
    float3 normal;
    float pad;
    float2 texcoord;
};

// バッファおよびテクスチャ定義
StructuredBuffer<VertexData> gVertexBuffer : register(t0);
StructuredBuffer<uint> indexBuffer : register(t1);
RWTexture2DArray<float4> outputShadowMap : register(u0);

// 各種定数バッファ
cbuffer Transform : register(b0)
{
    float4x4 matWorld;
}

struct Pointlight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    uint isHalf;
    uint castShadow;
};

static const int MAX_POINT_LIGHT = 32;
struct PointLights
{
    Pointlight lights[MAX_POINT_LIGHT];
    uint numLights;
};

cbuffer LightInfo : register(b1)
{
    PointLights gPointLights;
}

cbuffer ShadowMapInfo : register(b2)
{
    uint shadowMapSize;
    uint processLightStartIndex;
    uint processLightCount;
    float shadowBias;
}

// レイと三角形の交差判定
bool RayTriangleIntersect(float3 rayOrigin, float3 rayDir, float3 v0, float3 v1, float3 v2, out float t)
{
    float3 e1 = v1 - v0;
    float3 e2 = v2 - v0;
    float3 p = cross(rayDir, e2);
    float det = dot(e1, p);

    // カリング設定（両面の場合はコメントアウト）
    if (det < 1e-8)
    {
        t = 1e30;
        return false;
    }

    float invDet = 1.0 / det;
    float3 s = rayOrigin - v0;
    float u = dot(s, p) * invDet;

    if (u < 0 || u > 1)
    {
        t = 1e30;
        return false;
    }

    float3 q = cross(s, e1);
    float v = dot(rayDir, q) * invDet;

    if (v < 0 || u + v > 1)
    {
        t = 1e30;
        return false;
    }

    t = dot(e2, q) * invDet;
    return t > 0;
}

// キューブマップの面インデックスから方向ベクトルを計算
float3 GetCubemapDirection(uint faceIndex, float2 uv)
{
    // uvは0～1の範囲
    float2 texCoord = uv * 2.0 - 1.0; // -1～1の範囲に変換

    float3 dir;
    switch (faceIndex)
    {
        case 0:
            dir = float3(1.0, -texCoord.y, -texCoord.x);
            break; // +X
        case 1:
            dir = float3(-1.0, -texCoord.y, texCoord.x);
            break; // -X
        case 2:
            dir = float3(texCoord.x, 1.0, texCoord.y);
            break; // +Y
        case 3:
            dir = float3(texCoord.x, -1.0, -texCoord.y);
            break; // -Y
        case 4:
            dir = float3(texCoord.x, -texCoord.y, 1.0);
            break; // +Z
        case 5:
            dir = float3(-texCoord.x, -texCoord.y, -1.0);
            break; // -Z
        default:
            dir = float3(0, 0, 0);
            break;
    }

    return normalize(dir);
}

// メインコンピュートシェーダー関数
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // シャドウマップのサイズチェック
    if (DTid.x >= shadowMapSize || DTid.y >= shadowMapSize)
        return;

    // ライトと面のインデックス計算
    uint lightArrayIndex = DTid.z / 6; // 処理するライトの配列内インデックス
    uint actualLightIndex = processLightStartIndex + lightArrayIndex; // 実際のライトインデックス
    uint faceIndex = DTid.z % 6; // キューブマップの面インデックス

    // 指定された処理ライト数を超えていないかチェック
    if (lightArrayIndex >= processLightCount || actualLightIndex >= gPointLights.numLights)
        return;

    // 現在のポイントライト取得
    Pointlight currentLight = gPointLights.lights[actualLightIndex];

    // シャドウを生成しないライトの場合はスキップ
    if (currentLight.castShadow == 0)
    {
        outputShadowMap[DTid] = float4(0, 0, 0, 1);
        return;
    }

    // テクスチャ座標の計算（0～1の範囲）
    float2 uv = float2(DTid.x + 0.5, DTid.y + 0.5) / float(shadowMapSize);

    // キューブマップの面と方向の計算
    float3 rayDir = GetCubemapDirection(faceIndex, uv);

    // 光源位置
    float3 lightPos = currentLight.position;

    // 交差判定のためのパラメータ初期化
    float closestT = 1e30;
    uint hitObjectID = 0;

    // インデックスバッファのサイズ取得と三角形数計算
    uint numIndices = 0;
    uint stride = 0;
    indexBuffer.GetDimensions(numIndices, stride);
    uint numTriangles = numIndices / 3;

    // 全三角形との交差判定
    for (uint triIdx = 0; triIdx < numTriangles; triIdx++)
    {
        // インデックスから三角形の頂点を取得
        uint i0 = indexBuffer[triIdx * 3];
        uint i1 = indexBuffer[triIdx * 3 + 1];
        uint i2 = indexBuffer[triIdx * 3 + 2];

        // 頂点データ取得とワールド変換
        float3 v0 = mul(float4(gVertexBuffer[i0].position.xyz, 1.0), matWorld).xyz;
        float3 v1 = mul(float4(gVertexBuffer[i1].position.xyz, 1.0), matWorld).xyz;
        float3 v2 = mul(float4(gVertexBuffer[i2].position.xyz, 1.0), matWorld).xyz;

        // レイと三角形の交差判定
        float t;
        if (RayTriangleIntersect(lightPos, rayDir, v0, v1, v2, t))
        {
            if (t < closestT)
            {
                closestT = t;
                // ここではトライアングルインデックスをIDとして使用
                // 実際の実装ではオブジェクト固有のIDを使用すべき
                hitObjectID = triIdx + 1; // 0は「ヒットなし」のために予約
            }
        }
    }

    // 交差がなければ最大距離を設定
    if (closestT >= 1e30)
    {
        outputShadowMap[DTid] = float4(0, 0, 0, 1.0);
        return;
    }

    // IDをRGBに格納
    float r = (hitObjectID & 0xFF) / 255.0; // 下位8bit
    float g = ((hitObjectID >> 8) & 0xFF) / 255.0; // 中位8bit
    float b = ((hitObjectID >> 16) & 0xFF) / 255.0; // 上位8bit

    // 距離を正規化（0～1の範囲に）
    float normalizedDistance = min(closestT / currentLight.radius, 1.0);

    // 結果をテクスチャに書き込み
    outputShadowMap[DTid] = float4(r, g, b, normalizedDistance);
}