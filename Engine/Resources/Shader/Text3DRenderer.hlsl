// 3Dテキストレンダリング用シェーダー

struct VSInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

// カメラ情報
cbuffer CameraBuffer : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float3 cameraPosition;
}

// ワールド行列の配列
StructuredBuffer<float4x4> worldMatrices : register(t0);

VSOutput VSmain(VSInput input, uint vertexID : SV_VertexID)
{
    uint matrixIndex = vertexID / 6; // 6頂点で1文字を構成

    VSOutput output;

    // ワールド座標変換
    float4 worldPos = mul(input.position, worldMatrices[matrixIndex]);

    // ビュー座標変換
    float4 viewPos = mul(worldPos, viewMatrix);

    // プロジェクション座標変換
    output.position = mul(viewPos, projectionMatrix);

    output.texCoord = input.texCoord;
    output.color = input.color;

    return output;
}

// フォントテクスチャ
Texture2D<float> fontTexture : register(t1);
SamplerState fontSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

float4 PSmain(PSInput input) : SV_TARGET
{
    float alpha = fontTexture.Sample(fontSampler, input.texCoord);

    return float4(input.color.rgb, input.color.a * alpha);
}
