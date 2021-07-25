
#include "dx_local.h"



VertexShader* FullScreenTriangleVertexShader = nullptr;
PixelShader*  UvPixelShader = nullptr;

VertexShader* ImageDrawVertexShader = nullptr;
PixelShader*  ImageDrawPixelShader = nullptr;
PixelShader*  TiledImageDrawPixelShader = nullptr;
PixelShader*  ColorDrawPixelShader = nullptr;
PixelShader*  CharDrawPixelShader = nullptr;

VertexShader* SkyVertexShader = nullptr;
PixelShader*  SkyPixelShader = nullptr;

VertexShader* MeshVertexShader = nullptr;
PixelShader* MeshDebugPixelShader = nullptr;


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
		Macros.push_back({ L"DRAW2DSHADER", L"1" });
		ImageDrawVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ImageDrawVertexShader", &Macros);
		ImageDrawPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ImageDrawPixelShader", &Macros);
		TiledImageDrawPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"TiledImageDrawPixelShader", &Macros);
		ColorDrawPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ColorDrawPixelShader", &Macros);
		CharDrawPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"CharDrawPixelShader", &Macros);
	}
	{
		Macros Macros;
		SkyVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\SkyShader.hlsl", L"SkyVertexShader", &Macros);
		SkyPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\SkyShader.hlsl", L"SkyPixelShader", &Macros);
	}
	{
		Macros Macros;
		Macros.push_back({ L"TESTMESHSHADER", L"1" });
		MeshVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"MeshVertexShader", &Macros);
		MeshDebugPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"MeshDebugPixelShader", &Macros);
	}
}

void UnloadAllShaders()
{
	delete FullScreenTriangleVertexShader;
	delete UvPixelShader;

	delete ImageDrawVertexShader;
	delete ImageDrawPixelShader;
	delete TiledImageDrawPixelShader;
	delete ColorDrawPixelShader;
	delete CharDrawPixelShader;

	delete SkyVertexShader;
	delete SkyPixelShader;

	delete MeshVertexShader;
	delete MeshDebugPixelShader;
}


