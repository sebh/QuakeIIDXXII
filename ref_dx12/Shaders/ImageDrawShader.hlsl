


#include "StaticSamplers.hlsl"



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



#if COLORDRAWSHADER

cbuffer ImageDrawConstantBuffer : register(b0)
{
	float4	ColorAlpha;
}

float4 ColorDrawPixelShader() : SV_TARGET
{
	return ColorAlpha;
}

#endif // DRAW2DSHADER



#if DRAWIMAGESHADER

cbuffer ImageConstantBuffer	: register(b0)
{
	float2	OutputWidthAndInv;
	float2	OutputHeightAndInv;
}



struct VertexInput
{
	float2 Position			: POSITION;
	float2 SurfaceUV		: TEXCOORD0;
	float4 ColorAlpha		: TEXCOORD1;
};

struct VertexOutput
{
	float4 Position			: SV_POSITION;
	float2 SurfaceUV		: TEXCOORD0;
	float4 ColorAlpha		: TEXCOORD2;
};

Texture2D<float4> SurfaceTexture	: register(t0);



VertexOutput ColoredImageVertexShader(VertexInput Input, uint VertexID : SV_VertexID)
{
	VertexOutput output;

	output.Position = float4(Input.Position * float2(OutputWidthAndInv.y, OutputHeightAndInv.y) * float2(2.0f, -2.0f) + float2(-1.0, 1.0), 0.1, 1.0);
	output.SurfaceUV = Input.SurfaceUV;
	output.ColorAlpha = Input.ColorAlpha;

	return output;
}

float4 ColoredImagePixelShader(VertexOutput Input) : SV_TARGET
{
	return Input.ColorAlpha * SurfaceTexture.Sample(SamplerLinearRepeat, Input.SurfaceUV);
}

#endif // DRAWIMAGESHADER
