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

VSOutput VSmain(VSInput input)
{
    VSOutput output;
    output.position = input.position;
    output.texCoord = input.texCoord;
    output.color = input.color;
    return output;
}


Texture2D<float> fontTexture : register(t0);
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
