
#include "dx_local.h"




RenderTextureDynamic* Dx12Lightmaps[MAX_LIGHTMAPS];
byte* Dx12LightmapsCPUData[MAX_LIGHTMAPS];


#define LIGHTMAP_UNINITIALISED		0
#define LIGHTMAP_NEEDUPLOAD			1
#define LIGHTMAP_VALID				2
byte Dx12LightmapsStatus[MAX_LIGHTMAPS];



void R_InitLightmaps()
{
	for (int i = 0; i < MAX_LIGHTMAPS; ++i)
	{
		Dx12Lightmaps[i] = new RenderTextureDynamic(BLOCK_WIDTH, BLOCK_HEIGHT, 1, DXGI_FORMAT_R8G8B8A8_UNORM);

		Dx12LightmapsCPUData[i] = new byte[BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES];
	}
	R_ResetLightmaps();
}

void R_ResetLightmaps()
{
	// Reset all lightmap as not used
	memset(Dx12LightmapsStatus, LIGHTMAP_UNINITIALISED, sizeof(Dx12LightmapsStatus));
}

void dxglTexImage2DClearToBlack(uint LightmapIndex)
{
	ATLASSERT(LightmapIndex < MAX_LIGHTMAPS);

	memset(Dx12LightmapsCPUData[LightmapIndex], 0, BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
	Dx12LightmapsStatus[LightmapIndex] = LIGHTMAP_NEEDUPLOAD;
}

void dxglTexImage2D(uint LightmapIndex, const byte *pixels)
{
	ATLASSERT(LightmapIndex < MAX_LIGHTMAPS);

	memcpy(Dx12LightmapsCPUData[LightmapIndex], (const byte*)pixels, BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
	Dx12LightmapsStatus[LightmapIndex] = LIGHTMAP_NEEDUPLOAD;
}

void dxglTexSubImage2D(uint LightmapIndex, uint xoffset, uint yoffset, uint width, uint height, const byte *pixels)
{
	ATLASSERT(LightmapIndex < MAX_LIGHTMAPS);

//	xoffset = xoffset >= BLOCK_WIDTH  ? BLOCK_WIDTH -1 : xoffset;
//	yoffset = yoffset >= BLOCK_HEIGHT ? BLOCK_HEIGHT-1 : yoffset;
//	width  = (xoffset + width)  > BLOCK_WIDTH  ? BLOCK_WIDTH  - xoffset : width;
//	height = (yoffset + height) > BLOCK_HEIGHT ? BLOCK_HEIGHT - yoffset : height;

	uint xEnd = xoffset + width;
	uint yEnd = yoffset + height;
	for (uint x = xoffset; x < xEnd; ++x)
	{
		for (uint y = yoffset; y < yEnd; ++y)
		{
			uint iDst = y * BLOCK_WIDTH * LIGHTMAP_BYTES + x * LIGHTMAP_BYTES;
			uint iSrc = (y - yoffset) * width * LIGHTMAP_BYTES + (x - xoffset) * LIGHTMAP_BYTES;
			Dx12LightmapsCPUData[LightmapIndex][iDst + 0] = pixels[iSrc + 0];
			Dx12LightmapsCPUData[LightmapIndex][iDst + 1] = pixels[iSrc + 1];
			Dx12LightmapsCPUData[LightmapIndex][iDst + 2] = pixels[iSrc + 2];
			Dx12LightmapsCPUData[LightmapIndex][iDst + 3] = 1.0;// pixels[iSrc + 3];
		}
	}

	Dx12LightmapsStatus[LightmapIndex] = LIGHTMAP_NEEDUPLOAD;
}

void R_UploadLightmaps()
{
	for (int i = 0; i < MAX_LIGHTMAPS; ++i)
	{
		switch (Dx12LightmapsStatus[i])
		{
		case LIGHTMAP_UNINITIALISED:
			memset(Dx12LightmapsCPUData[i], 0, BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
			Dx12LightmapsStatus[i] = LIGHTMAP_VALID;	// Stop here, we only upload if some data has been specified
			break;
		case LIGHTMAP_NEEDUPLOAD:
			Dx12Lightmaps[i]->Upload(Dx12LightmapsCPUData[i], BLOCK_WIDTH * LIGHTMAP_BYTES, BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
			Dx12Lightmaps[i]->getRenderTexture().resourceTransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			Dx12LightmapsStatus[i] = LIGHTMAP_VALID;
			break;
		case LIGHTMAP_VALID:
			// nothing to do
			break;
		}
	}
}

void R_ShutdownLightmaps()
{
	for (int i = 0; i < MAX_LIGHTMAPS; ++i)
	{
		delete Dx12Lightmaps[i];
		delete [] Dx12LightmapsCPUData[i];
	}
}