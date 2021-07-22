


#include "StaticSamplers.hlsl"



cbuffer SkyConstantBuffer : register(b0)
{
	//float4x4	MeshWorldMatrix;
	float4x4	ViewProjectionMatrix;
	uint		Face;
	uint3		pad0;
}

Texture2D SkyFaceTexture : register(t0);

struct VertexOutput
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

VertexOutput SkyVertexShader(uint VertexID : SV_VertexID)
{
	VertexOutput output;	// TODO init to 0

	float2 Norm2d = float2((VertexID == 1 || VertexID == 3) ? 1.0 : -1.0, (VertexID == 2 || VertexID == 3) ? 1.0 : -1.0);

	// static char*	suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };
	float3 WorldPos = 0.0f;
	if (Face == 0)		// right
	{
		WorldPos = float3(1.0, Norm2d.x, -Norm2d.y);
	}
	else if (Face == 1)	// back
	{
		WorldPos = float3(Norm2d.x, -1.0, -Norm2d.y);
	}
	else if (Face == 2)	// left
	{
		WorldPos = float3(-1.0, -Norm2d.x, -Norm2d.y);
	}
	else if (Face == 3)	// front
	{
		WorldPos = float3(-Norm2d.x, 1.0, -Norm2d.y);
	}
	else if (Face == 4)	// Top
	{
		WorldPos = float3(Norm2d.y, Norm2d.x, 1.0);
	}
	else if (Face == 5)	// Bottom
	{
		WorldPos = float3(-Norm2d.y, Norm2d.x, -1.0);
	}
	WorldPos *= 2000.0f; // scale

	//output.position = mul(ViewProjectionMatrix, mul(MeshWorldMatrix, float4(WorldPos, 1.0)));
	output.position = mul(ViewProjectionMatrix, float4(WorldPos, 1.0));
	output.uv = Norm2d * 0.5f + 0.5f;

	return output;
}

float4 SkyPixelShader(VertexOutput input) : SV_TARGET
{
	return SkyFaceTexture.Sample(SamplerLinearClamp, float2(input.uv));
}


