
#include "dx_local.h"



VertexShader* TestVertexShader = nullptr;
PixelShader*  TestPixelShader = nullptr;

VertexShader* ImageDrawVertexShader = nullptr;
PixelShader*  ImageDrawPixelShader = nullptr;


void LoadAllShaders()
{
	{
		Macros Macros;
		Macros.push_back({L"TESTSHADER", L"1"});
		TestVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"TestVertexShader", &Macros);
		TestPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"TestPixelShader", &Macros);
	}
	{
		Macros Macros;
		Macros.push_back({ L"IMAGEDRAWSHADER", L"1" });
		ImageDrawVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ImageDrawVertexShader", &Macros);
		ImageDrawPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ImageDrawPixelShader", &Macros);
	}
}

void UnloadAllShaders()
{
	delete TestVertexShader;
	delete TestPixelShader;

	delete ImageDrawVertexShader;
	delete ImageDrawPixelShader;
}


