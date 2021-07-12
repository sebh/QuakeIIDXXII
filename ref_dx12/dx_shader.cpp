
#include "dx_local.h"



VertexShader* ImageDrawVertexShader = nullptr;
PixelShader*  ImageDrawPixelShader = nullptr;


void LoadAllShaders()
{
	Macros Macros;

	ImageDrawVertexShader = new VertexShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ImageDrawVertexShader", &Macros);
	ImageDrawPixelShader = new PixelShader(L"D:\\Projects\\Git\\QuakeIIDXXII\\ref_dx12\\Shaders\\ImageDrawShader.hlsl", L"ImageDrawPixelShader", &Macros);
}

void UnloadAllShaders()
{
	delete ImageDrawVertexShader;
	delete ImageDrawPixelShader;
}


