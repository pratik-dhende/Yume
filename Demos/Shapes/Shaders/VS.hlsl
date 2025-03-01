cbuffer cbPerObject : register(b0)
{
    float4x4 world;
};

cbuffer cbPerPass : register(b1)
{
    float4x4 view;
    float4x4 inverseView;
    
    float4x4 projection;
    float4x4 inverseProjection;
    
    float4x4 viewProjection;
    float4x4 inverseViewProjection;
    
    float3 cameraPositionW;
    float padding0;
    
    float2 renderTargetSize;
    float2 inverseRenderTargetSize;
    
    float nearZ;
    float farZ;
    
    float gTime;
    float gDeltaTime;
}

struct VertexIn
{
    float3 posL : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    
    vOut.posH = mul(float4(vIn.posL, 1.0f), mul(world, viewProjection));
    vOut.color = vIn.color;
    
    return vOut;
}