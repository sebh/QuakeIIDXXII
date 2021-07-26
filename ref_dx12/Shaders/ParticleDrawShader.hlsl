


#include "StaticSamplers.hlsl"



cbuffer ParticleConstantBuffer	: register(b0)
{
	float4x4	ViewProjectionMatrix;
	float4		ViewRight;
	float4		ViewUp;
	float		ParticleSize;
	float		Pad0[3];
}



struct VertexInput
{
	float3 Position			: POSITION;
	float4 ColorAlpha		: TEXCOORD0;
};

struct VertexOutput
{
	float4 Position			: SV_POSITION;
	float4 ColorAlpha		: TEXCOORD0;
	float2 UV				: TEXCOORD1;
};


VertexOutput ParticleVertexShader(VertexInput Input, uint VertexID : SV_VertexID)
{
	VertexOutput Output;

	float2 Norm2d = float2((VertexID == 1 || VertexID == 4 || VertexID == 5) ? 1.0 : 0.0, (VertexID == 2 || VertexID == 3 || VertexID == 5) ? 1.0 : 0.0) * 2.0f - 1.0f;
	const float3 PosOffset = ParticleSize * 0.5 * (Norm2d.x * ViewRight.xyz + Norm2d.y * ViewUp.xyz);

	Output.Position = mul(ViewProjectionMatrix, float4(Input.Position.xyz + PosOffset, 1.0f));
	Output.ColorAlpha = Input.ColorAlpha;
	Output.UV = Norm2d;

	return Output;
}

float4 ParticlePixelShader(VertexOutput Input) : SV_TARGET
{
	const float Gradient = dot(Input.UV,Input.UV);
	return float4(Input.ColorAlpha.rgb, Input.ColorAlpha.a * (Gradient > 1.0f ? 0.0f : saturate((1.0 - Gradient)*3.0f)));
}


