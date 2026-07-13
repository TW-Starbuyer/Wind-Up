Texture2D    albedo   : register(t0, space2);
SamplerState sampler0 : register(s0, space2);

struct PSInput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
    return albedo.Sample(sampler0, input.uv);
}