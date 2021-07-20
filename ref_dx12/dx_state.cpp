
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

void ErrorExit(char* Text)
{
	OutputDebugStringA(Text);
	ri.Sys_Error(ERR_DROP, Text);
}


