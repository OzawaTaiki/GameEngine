
/// VS関連
struct VertexInput
{
    float4 position[8];
    uint indexs[36];
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 texCoord : TEXCOORD0;
};

cbuffer TransformationMatrix : register(b0)
{
    float4x4 matWorld;
    float4x4 matWorldInverseTranspose;
};

cbuffer Camera : register(b1)
{
    float4x4 matView;
    float4x4 matProj;
    float3 worldPosition;
    float pad;
}

static const VertexInput kVertexData =
{
    // pos
    {
        float4(-1.0f, -1.0f,  1.0f, 1.0f),   // 0: 左下前
        float4(-1.0f,  1.0f,  1.0f, 1.0f),   // 1: 左上前
        float4( 1.0f,  1.0f,  1.0f, 1.0f),   // 2: 右上前
        float4( 1.0f, -1.0f,  1.0f, 1.0f),   // 3: 右下前
        float4(-1.0f, -1.0f, -1.0f, 1.0f),   // 4: 左下後
        float4(-1.0f,  1.0f, -1.0f, 1.0f),   // 5: 左上後 
        float4( 1.0f,  1.0f, -1.0f, 1.0f),   // 6: 右上後
        float4( 1.0f, -1.0f, -1.0f, 1.0f)    // 7: 右下後
    },
    // index
    {
        // 前面 (+Z)
        0, 1, 2,
        0, 2, 3,
        // 背面 (-Z)
        7, 6, 5,
        7, 5, 4,
        // 左面 (-X)
        4, 5, 1,
        4, 1, 0,
        // 右面 (+X)
        3, 2, 6,
        3, 6, 7,
        // 上面 (+Y)
        1, 5, 6,
        1, 6, 2,
        // 下面 (-Y)
        4, 0, 3,
        4, 3, 7
    }
};

VertexOutput VSmain(uint _id : SV_VertexID) // _idは頂点のインデックス
{
    VertexOutput output;
    uint index = kVertexData.indexs[_id];
    float4 pos = kVertexData.position[index];

    output.position = mul(pos, mul(matWorld, mul(matView, matProj))).xyww;
    output.texCoord = pos.xyz;

    return output;
};


// PS関連
TextureCube<float4> gSkyBoxTexture : register(t0);
SamplerState gSkyBoxSampler : register(s0);

struct PSOutput
{
    float4 color : SV_Target0;
};

cbuffer gColor : register(b2)
{
    float4 materialColor;
}

PSOutput PSmain(VertexOutput _input)
{
    PSOutput output;

    // テクスチャ座標を正規化
    float3 texCoord = normalize(_input.texCoord);

    // テクスチャサンプリング
    float4 textureColor = gSkyBoxTexture.Sample(gSkyBoxSampler, texCoord);

    // 色の計算
    output.color = materialColor * textureColor;

    return output;
}