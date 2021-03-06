
#include "dx_local.h"



VertexShader* FullScreenTriangleVertexShader = nullptr;
PixelShader*  UvPixelShader = nullptr;

PixelShader*  ColorDrawPixelShader = nullptr;

VertexShader* SkyVertexShader = nullptr;
PixelShader*  SkyPixelShader = nullptr;

VertexShader* MeshVertexShader = nullptr;
PixelShader* MeshDebugPixelShader = nullptr;
PixelShader* MeshColorPixelShader = nullptr;
PixelShader* MeshLightmapSurfacePixelShader = nullptr;
PixelShader* MeshColoredSurfacePixelShader = nullptr;

VertexShader* ColoredImageVertexShader = nullptr;
PixelShader* ColoredImagePixelShader = nullptr;


VertexShader* ParticleVertexShader = nullptr;
PixelShader* ParticlePixelShader = nullptr;

void LoadAllShaders()
{
	// D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders
	// C:\\Users\\Sebastien\\Projects\\QuakeIIDXXII\\ref_dx12\\Shaders
	{
		Macros Macros;
		Macros.push_back({L"TESTSHADER", L"1"});
		FullScreenTriangleVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"FullScreenTriangleVertexShader", &Macros);
		UvPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"UvPixelShader", &Macros);
	}
	{
		Macros Macros;
		Macros.push_back({ L"COLORDRAWSHADER", L"1" });
		ColorDrawPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ColorDrawPixelShader", &Macros);
	}
	{
		Macros Macros;
		SkyVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\SkyShader.hlsl", L"SkyVertexShader", &Macros);
		SkyPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\SkyShader.hlsl", L"SkyPixelShader", &Macros);
	}
	{
		Macros Macros;
		MeshVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\WorldMeshDrawShader.hlsl", L"MeshVertexShader", &Macros);
		MeshDebugPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\WorldMeshDrawShader.hlsl", L"MeshDebugPixelShader", &Macros);
		MeshColorPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\WorldMeshDrawShader.hlsl", L"MeshColorPixelShader", &Macros);
		MeshLightmapSurfacePixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\WorldMeshDrawShader.hlsl", L"MeshLightmapSurfacePixelShader", &Macros);
		MeshColoredSurfacePixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\WorldMeshDrawShader.hlsl", L"MeshColoredSurfacePixelShader", &Macros);
	}
	{
		Macros Macros;
		Macros.push_back({ L"DRAWIMAGESHADER", L"1" });
		ColoredImageVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ColoredImageVertexShader", &Macros);
		ColoredImagePixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ColoredImagePixelShader", &Macros);
	}
	{
		Macros Macros;
		ParticleVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ParticleDrawShader.hlsl", L"ParticleVertexShader", &Macros);
		ParticlePixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ParticleDrawShader.hlsl", L"ParticlePixelShader", &Macros);
	}
}

void UnloadAllShaders()
{
	delete FullScreenTriangleVertexShader;
	delete UvPixelShader;

	delete ColorDrawPixelShader;

	delete SkyVertexShader;
	delete SkyPixelShader;

	delete MeshVertexShader;
	delete MeshDebugPixelShader;
	delete MeshColorPixelShader;
	delete MeshLightmapSurfacePixelShader;
	delete MeshColoredSurfacePixelShader;

	delete ColoredImageVertexShader;
	delete ColoredImagePixelShader;

	delete ParticleVertexShader;
	delete ParticlePixelShader;
}


