


#include "StaticSamplers.hlsl"



#if TESTMESHSHADER

cbuffer MeshConstantBuffer : register(b0)
{
	float4x4	MeshWorldMatrix;
	float4x4	ViewProjectionMatrix;
}

struct VertexInput
{
	float3 position		: POSITION;
};
struct VertexOutput
{
	float4 position		: SV_POSITION;
};

VertexOutput MeshVertexShader(VertexInput input, uint VertexID : SV_VertexID)
{
	VertexOutput output;

	output.position = mul(ViewProjectionMatrix, mul(MeshWorldMatrix, float4(input.position, 1.0)));

	return output;
}

float4 MeshDebugPixelShader(VertexOutput input) : SV_TARGET
{
	return float4(1.0, 0.5, 0.25, 1.0);
}

#endif // TESTMESHSHADER



#if TESTSHADER

struct VertexOutput
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

VertexOutput FullScreenTriangleVertexShader(uint VertexID : SV_VertexID)
{
	VertexOutput output;	// TODO init to 0

	output.position = float4(VertexID == 2 ? 3.0 : -1.0, VertexID == 1 ? 3.0 : -1.0, 0.1, 1.0);
	output.uv = (output.position.xy + 1.0) * 0.25;

	return output;
}

float4 UvPixelShader(VertexOutput input) : SV_TARGET
{
	return float4(input.uv, 0, 0);
}

#endif // TESTSHADER



#if DRAW2DSHADER

cbuffer ImageDrawConstantBuffer : register(b0)
{
	float2	OutputWidthAndInv;
	float2	OutputHeightAndInv;
	float2	ImageBottomLeft;
	float2	ImageSize;
	float4	ColorAlpha;
}

Texture2D ImageTexture : register(t0);

struct VertexOutput
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

VertexOutput ImageDrawVertexShader(uint VertexID : SV_VertexID)
{
	VertexOutput output;	// TODO init to 0

	float2 Norm2d = float2((VertexID == 1 || VertexID == 3) ? 1.0 : 0.0, (VertexID == 2 || VertexID == 3) ? 1.0 : 0.0);

	output.position = float4((ImageBottomLeft + Norm2d * ImageSize) * float2(OutputWidthAndInv.y, OutputHeightAndInv.y) * float2(2.0f, -2.0f) + float2(-1.0, 1.0), 0.1, 1.0);
	output.uv = Norm2d;

	return output;
}

float4 ImageDrawPixelShader(VertexOutput input) : SV_TARGET
{
	return ImageTexture.Sample(SamplerLinearClamp, float2(input.uv));
}

float4 TiledImageDrawPixelShader(VertexOutput input) : SV_TARGET
{
	return ImageTexture.Sample(SamplerPointRepeat, input.position.xy / 64.0f);
}

float4 ColorDrawPixelShader(VertexOutput input) : SV_TARGET
{
	return ColorAlpha;
}

float4 CharDrawPixelShader(VertexOutput input) : SV_TARGET
{
	const float2 UvOffset = ColorAlpha.xy;
	const float2 UvSize = ColorAlpha.zw;
	return ImageTexture.Sample(SamplerPointRepeat, input.uv * UvSize + UvOffset);
}

#endif // DRAW2DSHADER