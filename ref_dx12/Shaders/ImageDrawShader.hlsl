


#include "StaticSamplers.hlsl"



struct VertexOutput
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

VertexOutput ImageDrawVertexShader(uint VertexID : SV_VertexID)
{
	VertexOutput output;	// TODO init to 0

	output.position = float4(VertexID == 2 ? 3.0 : -1.0, VertexID == 1 ? 3.0 : -1.0, 0.1, 1.0);
	output.uv = (output.position.xy + 1.0) * 0.25;

	return output;
}

float4 ImageDrawPixelShader(VertexOutput input) : SV_TARGET
{
	return float4(input.uv, 0, 0);
}


