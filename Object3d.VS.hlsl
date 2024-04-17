cbuffer TrancsFormationMatrix : register(b0)
{
    float4x4 WVP;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
};

struct VertexShaderInput
{
    //float32_t4 position : POSITION0;
    float4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, WVP);
    return output;
}



