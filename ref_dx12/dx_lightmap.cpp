
#include "dx_local.h"




RenderTextureDynamic* Dx12Lightmaps[ALL_LIGHTMAPS];
byte* Dx12LightmapsCPUData[ALL_LIGHTMAPS];


#define LIGHTMAP_UNINITIALISED		0x00
#define LIGHTMAP_NEEDUPLOAD			0x01
#define LIGHTMAP_VALID				0x02
byte Dx12LightmapsStatus[ALL_LIGHTMAPS];
byte Dx12LightmapsNotOnlyCleared[ALL_LIGHTMAPS];



void R_InitLightmaps()
{
	for (int i = 0; i < ALL_LIGHTMAPS; ++i)
	{
		Dx12Lightmaps[i] = new RenderTextureDynamic(BLOCK_WIDTH, BLOCK_HEIGHT, 1, DXGI_FORMAT_R8G8B8A8_UNORM);

		Dx12LightmapsCPUData[i] = new byte[BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES];
		memset(Dx12LightmapsCPUData[i], 0, BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
	}
	R_ResetLightmaps();
}

void R_ResetLightmaps()
{
	// Reset all lightmap as not used
	memset(Dx12LightmapsStatus, LIGHTMAP_UNINITIALISED, sizeof(Dx12LightmapsStatus));
	memset(Dx12LightmapsNotOnlyCleared, 0, sizeof(Dx12LightmapsNotOnlyCleared));
}

void dxglTexImage2DClearToBlack(uint LightmapIndex)
{
	ATLASSERT(LightmapIndex < ALL_LIGHTMAPS);

	memset(Dx12LightmapsCPUData[LightmapIndex], 0, BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
	Dx12LightmapsStatus[LightmapIndex] = LIGHTMAP_NEEDUPLOAD;
}

void dxglTexImage2D(uint LightmapIndex, const byte *pixels)
{
	ATLASSERT(LightmapIndex < ALL_LIGHTMAPS);

	memcpy(Dx12LightmapsCPUData[LightmapIndex], (const byte*)pixels, BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
	Dx12LightmapsStatus[LightmapIndex] = LIGHTMAP_NEEDUPLOAD;
	Dx12LightmapsNotOnlyCleared[LightmapIndex] = 1;
}

void dxglTexSubImage2D(uint LightmapIndex, uint xoffset, uint yoffset, uint width, uint height, const byte *pixels)
{
	ATLASSERT(LightmapIndex < ALL_LIGHTMAPS);

	for (uint y = 0; y < height; ++y)
	{
		for (uint x = 0; x < width; ++x)
		{
			uint iDst = ((yoffset + y) * BLOCK_WIDTH + (xoffset + x)) * LIGHTMAP_BYTES;
			uint iSrc = (y * width + x) * LIGHTMAP_BYTES;
			Dx12LightmapsCPUData[LightmapIndex][iDst + 0] = pixels[iSrc + 0];
			Dx12LightmapsCPUData[LightmapIndex][iDst + 1] = pixels[iSrc + 1];
			Dx12LightmapsCPUData[LightmapIndex][iDst + 2] = pixels[iSrc + 2];
			Dx12LightmapsCPUData[LightmapIndex][iDst + 3] = 1.0;// pixels[iSrc + 3];
		}
	}

	Dx12LightmapsStatus[LightmapIndex] = LIGHTMAP_NEEDUPLOAD;
	Dx12LightmapsNotOnlyCleared[LightmapIndex] = 1;
}

void R_UploadLightmaps()
{
	for (int i = 0; i < ALL_LIGHTMAPS; ++i)
	{
		switch (Dx12LightmapsStatus[i])
		{
		case LIGHTMAP_UNINITIALISED:
			memset(Dx12LightmapsCPUData[i], 0, BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
			Dx12LightmapsStatus[i] = LIGHTMAP_VALID;
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


	if (dx_showlightmaps->value > 0)
	{
		DrawImageCall DrawImage;
		DrawImageCall dic;
		dic.Type = DrawImageCallType::Draw_Tex;
		dic.x = 0;
		dic.y = BLOCK_HEIGHT;
		dic.w = BLOCK_WIDTH;
		dic.h = BLOCK_HEIGHT;
		const int DebugPrintWidth = r_newrefdef.width / 2;
		for (int i = 0; i < MAX_LIGHTMAPS; ++i)
		{
			// Only print lightmap with real data (not only cleared)
			if (Dx12LightmapsNotOnlyCleared[i] == 0)
				continue;

			while ((dic.x + dic.w) > DebugPrintWidth)
			{
				dic.x = dic.x - DebugPrintWidth;
				if (dic.x < 0)
				{
					dic.x = 0;
				}
				dic.y += BLOCK_HEIGHT * 2 + 8;
			}

			DrawImage = dic;
			DrawImage.Texture = &Dx12Lightmaps[i]->getRenderTexture();
			AddDrawImage(DrawImage);

			if (Dx12LightmapsNotOnlyCleared[MAX_LIGHTMAPS + i] == 0)
			{
				dic.x += BLOCK_WIDTH + 4; // next position
				continue;
			}

			dic.y += BLOCK_HEIGHT + 2;
			DrawImage = dic;
			DrawImage.Texture = &Dx12Lightmaps[i + MAX_LIGHTMAPS]->getRenderTexture();
			AddDrawImage(DrawImage);
			dic.y -= BLOCK_HEIGHT + 2;

			dic.x += BLOCK_WIDTH + 4; // next position
		}
	}
}

void R_ShutdownLightmaps()
{
	for (int i = 0; i < ALL_LIGHTMAPS; ++i)
	{
		delete Dx12Lightmaps[i];
		delete [] Dx12LightmapsCPUData[i];
	}
}