struct VSInput
{
    float3 position : TEXCOORD0;
    float3 normal   : TEXCOORD1;
    float2 uv       : TEXCOORD2;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

cbuffer UBO : register(b0, space1)
{
    float4x4 mvp;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(mvp, float4(input.position, 1.0));
    output.uv       = input.uv;
    return output;
}