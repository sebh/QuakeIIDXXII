


#include "StaticSamplers.hlsl"



#if TESTSHADER

struct VertexOutput
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

VertexOutput TestVertexShader(uint VertexID : SV_VertexID)
{
	VertexOutput output;	// TODO init to 0

	output.position = float4(VertexID == 2 ? 3.0 : -1.0, VertexID == 1 ? 3.0 : -1.0, 0.1, 1.0);
	output.uv = (output.position.xy + 1.0) * 0.25;

	return output;
}

float4 TestPixelShader(VertexOutput input) : SV_TARGET
{
	return float4(input.uv, 0, 0);
}

#endif // TESTSHADER



#if IMAGEDRAWSHADER

cbuffer ImageDrawConstantBuffer : register(b0)
{
	float2	OutputWidthAndInv;
	float2	OutputHeightAndInv;
	float2	ImageBottomLeft;
	float2	ImageSize;
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

	float2 Norm2d = float2(VertexID == 2 ? 1.0 : 0.0, VertexID == 1 ? 1.0 : 0.0);

	output.position = float4((ImageBottomLeft + Norm2d * ImageSize) * float2(OutputWidthAndInv.y, OutputHeightAndInv.y), 0.1, 1.0);
	output.uv = Norm2d;

	return output;
}

float4 ImageDrawPixelShader(VertexOutput input) : SV_TARGET
{
	return ImageTexture.Sample(SamplerLinearClamp, float2(input.uv));
	//return float4(input.uv, 1, 0);
}

#endif // IMAGEDRAWSHADER