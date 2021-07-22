
#include "dx_local.h"

static bool bAllStatesCreated = false;

RenderBufferGeneric* IndexBufferSingleTri = nullptr;
RenderBufferGeneric* IndexBufferQuadTris = nullptr;

bool AreAllStatesCreated()
{
	return bAllStatesCreated;
}

void CreateAllStates()
{
	{
		uint Indices[3];
		Indices[0] = 0;
		Indices[1] = 1;
		Indices[2] = 2;
		IndexBufferSingleTri = new RenderBufferGeneric(3 * sizeof(uint), Indices);
		IndexBufferSingleTri->setDebugName(L"TriangleIndexBuffer");
	}
	{
		uint Indices[6];
		Indices[0] = 0;
		Indices[1] = 1;
		Indices[2] = 2;
		Indices[3] = 2;
		Indices[4] = 1;
		Indices[5] = 3;
		IndexBufferQuadTris = new RenderBufferGeneric(6 * sizeof(uint), Indices);
		IndexBufferQuadTris->setDebugName(L"QuadTrisIndexBuffer");
	}

	bAllStatesCreated = true;
}


void ReleaseAllStates()
{
	if (!bAllStatesCreated)
		return;

	delete IndexBufferSingleTri;
	delete IndexBufferQuadTris;
}



////////////////////////////////////////////////////////////////////////////////////////////////////

ViewData GetViewData()
{
	float AspectRatioXOverY = float(r_newrefdef.width) / float(r_newrefdef.height);

	ViewData View;

	View.ViewMatrix = XMMatrixIdentity();
	float4 ViewViewMatrixDet = XMMatrixDeterminant(View.ViewMatrix);
	View.ViewMatrixInv = XMMatrixInverse(&ViewViewMatrixDet, View.ViewMatrix);

	View.ProjectionMatrix = XMMatrixPerspectiveFovLH(90.0f*3.14159f / 180.0f, AspectRatioXOverY, 0.1f, 20000.0f);
	float4 ViewProjectionMatrixDet = XMMatrixDeterminant(View.ProjectionMatrix);
	View.ProjectionMatrixInv = XMMatrixInverse(&ViewProjectionMatrixDet, View.ProjectionMatrix);


	View.ViewProjectionMatrix = XMMatrixMultiply(View.ViewMatrix, View.ProjectionMatrix);
	float4 ViewViewProjectionMatrixDet = XMMatrixDeterminant(View.ViewProjectionMatrix);
	View.ViewProjectionMatrixInv = XMMatrixInverse(&ViewViewProjectionMatrixDet, View.ViewProjectionMatrix);

	return View;
}



////////////////////////////////////////////////////////////////////////////////////////////////////

void ErrorExit(char* Text)
{
	OutputDebugStringA(Text);
	ri.Sys_Error(ERR_DROP, Text);
}


