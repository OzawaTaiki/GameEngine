
struct VSInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float4 vColor   : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 vColor : COLOR;
    uint instanceID : SV_InstanceID;
};

struct InstanceData
{
    float4x4 affineMat;
    float4x4 uvTransform;
    float4 color;

    uint textureIndex; // SRV
    uint useTextureAlpha; // 0ならRをalphaとする(テキスト) 1ならRGBAを使う(スプライト)
    uint baseInstanceIndex;

    int pad;

};

StructuredBuffer<InstanceData> instanceData : register(t0);
cbuffer ViewProjection : register(b0)
{
    float4x4 orthoMat;
}

Texture2D<float4> textureArray[] : register(t1); // テクスチャ配列
SamplerState gSampler : register(s0);


VSOutput VSMain(VSInput input, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    InstanceData data = instanceData[instanceID];

    output.position = mul(input.position, mul(data.affineMat, orthoMat));
    output.texCoord = input.texCoord;
    output.vColor = data.color;
    output.instanceID = instanceID;

    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 output;

    InstanceData data = instanceData[input.instanceID];

    float2 transedUV = mul(float4(input.texCoord, 0.0f, 1.0f), data.uvTransform).xy;

    float4 texColor = textureArray[data.textureIndex].Sample(gSampler, transedUV);

    // useTextureAlpha が 1 なら、R成分をアルファとして扱う（テキスト）
    // useTextureAlpha が 0 なら、通常のRGBA（スプライト）
    float alpha = lerp(texColor.a, texColor.r, data.useTextureAlpha);
    float3 rgb = lerp(texColor.rgb, float3(1.0, 1.0, 1.0), data.useTextureAlpha);

    return float4(rgb * input.vColor.rgb * data.color.rgb,
                  alpha * input.vColor.a * data.color.a);
}