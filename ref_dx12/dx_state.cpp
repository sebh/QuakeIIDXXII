
#include "dx_local.h"


InputLayout* NullInputLayout = nullptr;


void CreateAllStates()
{
	NullInputLayout = new InputLayout();



	//extern RenderBufferGeneric* vertexBuffer;
	//extern RenderBufferGeneric* indexBuffer;
}


void ReleaseAllStates()
{
	delete NullInputLayout;
}
