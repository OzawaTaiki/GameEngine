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

cbuffer ViewProjection : register(b0)
{
    float4x4 orthoMat;
}
StructuredBuffer<float4x4> worldMatrices : register(t0);

VSOutput VSmain(VSInput input, uint vertexID : SV_VertexID)
{
    uint matrixIndex = vertexID / 6; // 頂点番号からワールド行列のインデックスを計算 六頂点で構成している

    VSOutput output;
    output.position = mul(input.position, mul(worldMatrices[matrixIndex], orthoMat));
    output.texCoord = input.texCoord;
    output.color = input.color;
    return output;
}


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

    if (input.color.a * alpha == 0)
        return float4(1, 0, 0, 1); // 赤で確認


    return float4(input.color.rgb, input.color.a * alpha);
}
