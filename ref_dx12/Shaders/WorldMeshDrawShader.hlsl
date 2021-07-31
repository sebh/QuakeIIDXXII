


#include "StaticSamplers.hlsl"



cbuffer MeshConstantBuffer	: register(b0)
{
	float4x4	MeshWorldMatrix;
	float4x4	ViewProjectionMatrix;
	float4		BatchDebugColor;
}



struct VertexInput
{
	float3 Position			: POSITION;
	float2 SurfaceUV		: TEXCOORD0;
	float2 LightmapUV		: TEXCOORD1;
	float4 ColorAlpha		: TEXCOORD2;
};

struct VertexOutput
{
	float4 Position			: SV_POSITION;
	float2 SurfaceUV		: TEXCOORD0;
	float2 LightmapUV		: TEXCOORD1;
	float4 ColorAlpha		: TEXCOORD2;
};


Texture2D<float4> SurfaceTexture	: register(t0);
Texture2D<float4> LightmapTexture	: register(t1);


VertexOutput MeshVertexShader(VertexInput Input, uint VertexID : SV_VertexID)
{
	VertexOutput Output;

	Output.Position = mul(ViewProjectionMatrix, mul(MeshWorldMatrix, float4(Input.Position, 1.0)));
	Output.SurfaceUV = Input.SurfaceUV;
	Output.LightmapUV = Input.LightmapUV;
	Output.ColorAlpha = Input.ColorAlpha;

	return Output;
}

float4 MeshDebugPixelShader(VertexOutput Input) : SV_TARGET
{
	return float4(1.0, 0.5, 0.25, 1.0);
}

float4 MeshColorPixelShader(VertexOutput Input) : SV_TARGET
{
	return lerp(
		Input.ColorAlpha,
		BatchDebugColor, BatchDebugColor.a);
}

float4 MeshLightmapSurfacePixelShader(VertexOutput Input) : SV_TARGET
{
	return lerp(
		float4(LightmapTexture.Sample(SamplerLinearClamp, Input.LightmapUV).rgb, 1.0f) * SurfaceTexture.Sample(SamplerLinearRepeat, Input.SurfaceUV),
		BatchDebugColor, BatchDebugColor.a);
}

float4 MeshColoredSurfacePixelShader(VertexOutput Input) : SV_TARGET
{
	return lerp(
		Input.ColorAlpha * SurfaceTexture.Sample(SamplerLinearRepeat, Input.SurfaceUV),
		BatchDebugColor, BatchDebugColor.a);
}


