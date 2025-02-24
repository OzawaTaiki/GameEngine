

struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInversedTransposeMatrix;
};

struct Vertex
{
    float4 postion;
    float3 normal;
    float2 texcoord;
};

struct VertexInfluence
{
    float4 weight;
    int4 index;
};

struct Skinning
{
    uint numVertices;
};

StructuredBuffer<Well> gMatrixPalette : register(t0);
StructuredBuffer<Vertex> gInputVertices : register(t1);
StructuredBuffer<VertexInfluence> gInfluences : register(t2);
RWStructuredBuffer<Vertex> gOutputVertices : register(u0);
cbuffer gSkinnedInformation : register(b0)
{
    Skinning skinnedInformation;
}

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;
    if (vertexIndex < skinnedInformation.numVertices)
    {
        Vertex input = gInputVertices[vertexIndex];
        VertexInfluence influence = gInfluences[vertexIndex];

        Vertex skinned;

        skinned.texcoord = input.texcoord;

        skinned.postion = mul(input.postion, gMatrixPalette[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
        skinned.postion += mul(input.postion, gMatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
        skinned.postion += mul(input.postion, gMatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
        skinned.postion += mul(input.postion, gMatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
        skinned.postion.w = 1.0f;

        skinned.normal = mul(input.normal, (float3x3) gMatrixPalette[influence.index.x].skeletonSpaceInversedTransposeMatrix) * influence.weight.x;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.y].skeletonSpaceInversedTransposeMatrix) * influence.weight.y;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.z].skeletonSpaceInversedTransposeMatrix) * influence.weight.z;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.w].skeletonSpaceInversedTransposeMatrix) * influence.weight.w;
        skinned.normal = normalize(skinned.normal);

        gOutputVertices[vertexIndex] = skinned;

    }
}
