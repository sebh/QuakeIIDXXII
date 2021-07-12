
#include "dx_local.h"

static bool bAllStatesCreated = false;

InputLayout* NullInputLayout = nullptr;

RenderBufferGeneric* IndexBuffer = nullptr;

bool AreAllStatesCreated()
{
	return bAllStatesCreated;
}

void CreateAllStates()
{
	NullInputLayout = new InputLayout();

	uint Indices[3];
	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 2;
	IndexBuffer = new RenderBufferGeneric(3 * sizeof(uint), Indices);
	IndexBuffer->setDebugName(L"TriangleIndexBuffer");

	bAllStatesCreated = true;
}


void ReleaseAllStates()
{
	if (!bAllStatesCreated)
		return;

	delete NullInputLayout;

	delete IndexBuffer;
}
