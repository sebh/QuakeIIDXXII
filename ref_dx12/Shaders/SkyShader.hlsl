


#include "StaticSamplers.hlsl"



cbuffer ImageDrawConstantBuffer : register(b0)
{
	float4x4	MeshWorldMatrix;
	float4x4	ViewProjectionMatrix;
	float4x4	ViewProjectionMatrixInv;
	uint		Face;
	uint3		pad0;
}

Texture2D SkyFaceTexture : register(t0);

struct VertexOutput
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

VertexOutput ImageDrawVertexShader(uint VertexID : SV_VertexID)
{
	VertexOutput output;	// TODO init to 0

	float2 Norm2d = float2((VertexID == 1 || VertexID == 3) ? 1.0 : -1.0, (VertexID == 2 || VertexID == 3) ? 1.0 : -1.0);

	// static char*	suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };
	float3 WorldPos = 0.0f;
	if (Face == 0)
	{
		WorldPos = float3(1.0, Norm2d.x, Norm2d.y);
	}
	else if (Face == 1)
	{
		WorldPos = float3(Norm2d.x, -1.0, Norm2d.y);
	}
	else if (Face == 2)
	{
		WorldPos = float3(-1.0, Norm2d.x, Norm2d.y);
	}
	else if (Face == 3)
	{
		WorldPos = float3(Norm2d.x, 1.0, Norm2d.y);
	}
	else if (Face == 4)
	{
		WorldPos = float3(Norm2d.x, Norm2d.y, 1.0);
	}
	else if (Face == 5)
	{
		WorldPos = float3(Norm2d.x, Norm2d.y, -1.0);
	}
	WorldPos *= 1000.0f; // scale

	output.position = mul(ViewProjectionMatrix, mul(MeshWorldMatrix, float4(WorldPos, 1.0)));
	output.uv = Norm2d;

	return output;
}

float4 ImageDrawPixelShader(VertexOutput input) : SV_TARGET
{
	return ImageTexture.Sample(SamplerLinearClamp, float2(input.uv));
}


