
#include "dx_local.h"


int c_brush_polys, c_alias_polys;
int	r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

model_t		*currentmodel;
entity_t	*currententity;



void R_SetLightLevel(void)
{
	vec3_t		shadelight;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// save off light value for server to look at (BIG HACK!)

	R_LightPoint(r_newrefdef.vieworg, shadelight);

	// pick the greatest component, which should be the same
	// as the mono value returned by software
	if (shadelight[0] > shadelight[1])
	{
		if (shadelight[0] > shadelight[2])
			r_lightlevel->value = 150 * shadelight[0];
		else
			r_lightlevel->value = 150 * shadelight[2];
	}
	else
	{
		if (shadelight[1] > shadelight[2])
			r_lightlevel->value = 150 * shadelight[1];
		else
			r_lightlevel->value = 150 * shadelight[2];
	}

}



void R_DrawEntitiesOnList()
{
	int		i;

	//	if (!r_drawentities->value)
	//		return;

		// draw non-transparent first
	for (i = 0; i < r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		if (currententity->flags & RF_TRANSLUCENT)
			continue;	// solid

		if (currententity->flags & RF_BEAM)
		{
			//R_DrawBeam(currententity);
		}
		else
		{
			currentmodel = currententity->model;
			if (!currentmodel)
			{
				//R_DrawNullModel();
				continue;
			}
			switch (currentmodel->type)
			{
			case mod_alias:
				//R_DrawAliasModel(currententity);
				break;
			case mod_brush:
				//R_DrawBrushModel(currententity);
				break;
			case mod_sprite:
				//R_DrawSpriteModel(currententity);
				break;
			default:
				ri.Sys_Error(ERR_DROP, "Bad modeltype");
				break;
			}
		}
	}

	// draw transparent entities
	// we could sort these if it ever becomes a problem...
//	qglDepthMask(0);		// no z writes
	for (i = 0; i < r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		if (!(currententity->flags & RF_TRANSLUCENT))
			continue;	// solid

		if (currententity->flags & RF_BEAM)
		{
			//R_DrawBeam(currententity);
		}
		else
		{
			currentmodel = currententity->model;

			if (!currentmodel)
			{
				//R_DrawNullModel();
				continue;
			}
			switch (currentmodel->type)
			{
			case mod_alias:
				//R_DrawAliasModel(currententity);
				break;
			case mod_brush:
				//R_DrawBrushModel(currententity);
				break;
			case mod_sprite:
				//R_DrawSpriteModel(currententity);
				break;
			default:
				ri.Sys_Error(ERR_DROP, "Bad modeltype");
				break;
			}
		}
	}
	//qglDepthMask(1);		// back to writing

}



// Render the world
// See https://fabiensanglard.net/quake2/quake2_opengl_renderer.php
void R_RenderView(void)
{
	SCOPED_GPU_TIMER(Quake2Frame, 100, 100, 100);

	ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
	ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();

	// Set the view port region
	D3D12_VIEWPORT Viewport;
	Viewport.TopLeftX = r_newrefdef.x;
	Viewport.TopLeftY = r_newrefdef.y;
	Viewport.Width = r_newrefdef.width;
	Viewport.Height = r_newrefdef.height;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	CommandList->RSSetViewports(1, &Viewport);
	D3D12_RECT ScissorRect;
	ScissorRect.left = r_newrefdef.x;
	ScissorRect.top = r_newrefdef.y;
	ScissorRect.right = r_newrefdef.x + r_newrefdef.width;
	ScissorRect.bottom = r_newrefdef.y + r_newrefdef.height;
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Clear the viewport for rendering
	FLOAT BackBufferClearColor[4] = { 0.0f, 0.5f, 0.0f, 1.0f };
	CommandList->ClearRenderTargetView(BackBufferDescriptor, BackBufferClearColor, 1, &ScissorRect);
	// TODO also clear depth

	if (r_speeds->value)
	{
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	// Mark polygon affected by dynamic light
	R_PushDlights();

	//TODO	R_SetupFrame();

	//TODO	R_SetFrustum();

	//TODO	R_SetupGL();

	// Decompress the PVS and mark potentially Visible Polygons
	R_MarkLeaves();

	//TODO	R_DrawWorld();

	// Render entities on top of the world
	R_DrawEntitiesOnList();

	// Blend dynamic lights. Used instead of lightmap uppdate. 
	// ==> Unused. Does not look good.
	//R_RenderDlights();

//TODO	R_DrawParticles();

		// Alpha blend translucent surfaces
//TODO	R_DrawAlphaSurfaces();

		// Post effects (full screen red for damage, etc...)
//TODO	R_Flash();

	if (r_speeds->value)
	{
		ri.Con_Printf(PRINT_ALL, "%4i wpoly %4i epoly %i tex %i lmaps\n",
			c_brush_polys,
			c_alias_polys,
			c_visible_textures,
			c_visible_lightmaps);
	}

	// Last render the sky
	SkyRender();

	R_SetLightLevel();
}



