
#include "dx_local.h"


int c_brush_polys, c_alias_polys;
int	r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

float		v_blend[4];			// final blending color

cplane_t	frustum[4];

model_t		*currentmodel;
entity_t	*currententity;

RenderBufferGenericDynamic* MeshRenderBuffer = nullptr;


void R_SetupFrame(void)
{
	int i;
	mleaf_t	*leaf;

	r_framecount++;

	// build the transformation matrix for the given view angles
	VectorCopy(r_newrefdef.vieworg, r_origin);

	AngleVectors(r_newrefdef.viewangles, vpn, vright, vup);

	// current viewcluster
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf(r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;

		// check above and below so crossing solid water doesn't draw wrong
		if (!leaf->contents)
		{	// look down a bit
			vec3_t	temp;

			VectorCopy(r_origin, temp);
			temp[2] -= 16;
			leaf = Mod_PointInLeaf(temp, r_worldmodel);
			if (!(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2))
				r_viewcluster2 = leaf->cluster;
		}
		else
		{	// look up a bit
			vec3_t	temp;

			VectorCopy(r_origin, temp);
			temp[2] += 16;
			leaf = Mod_PointInLeaf(temp, r_worldmodel);
			if (!(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2))
				r_viewcluster2 = leaf->cluster;
		}
	}

	for (i = 0; i < 4; i++)
		v_blend[i] = r_newrefdef.blend[i];

	c_brush_polys = 0;
	c_alias_polys = 0;

	// clear out the portion of the screen that the NOWORLDMODEL defines
	//if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
	//{
	//	qglEnable(GL_SCISSOR_TEST);
	//	qglClearColor(0.3, 0.3, 0.3, 1);
	//	qglScissor(r_newrefdef.x, vid.height - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height);
	//	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//	qglClearColor(1, 0, 0.5, 0.5);
	//	qglDisable(GL_SCISSOR_TEST);
	//}
}

int SignbitsForPlane(cplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j = 0; j < 3; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1 << j;
	}
	return bits;
}

void R_SetFrustum(void)
{
	int		i;

#if 0
	/*
	** this code is wrong, since it presume a 90 degree FOV both in the
	** horizontal and vertical plane
	*/
	// front side is visible
	VectorAdd(vpn, vright, frustum[0].normal);
	VectorSubtract(vpn, vright, frustum[1].normal);
	VectorAdd(vpn, vup, frustum[2].normal);
	VectorSubtract(vpn, vup, frustum[3].normal);

	// we theoretically don't need to normalize these vectors, but I do it
	// anyway so that debugging is a little easier
	VectorNormalize(frustum[0].normal);
	VectorNormalize(frustum[1].normal);
	VectorNormalize(frustum[2].normal);
	VectorNormalize(frustum[3].normal);
#else
	// rotate VPN right by FOV_X/2 degrees
	RotatePointAroundVector(frustum[0].normal, vup, vpn, -(90 - r_newrefdef.fov_x / 2));
	// rotate VPN left by FOV_X/2 degrees
	RotatePointAroundVector(frustum[1].normal, vup, vpn, 90 - r_newrefdef.fov_x / 2);
	// rotate VPN up by FOV_X/2 degrees
	RotatePointAroundVector(frustum[2].normal, vright, vpn, 90 - r_newrefdef.fov_y / 2);
	// rotate VPN down by FOV_X/2 degrees
	RotatePointAroundVector(frustum[3].normal, vright, vpn, -(90 - r_newrefdef.fov_y / 2));
#endif

	for (i = 0; i < 4; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct(r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane(&frustum[i]);
	}
}

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

void R_Flash()
{
	if (!gl_polyblend->value)
		return;
	if (!v_blend[3])
		return;

	FrameConstantBuffers& ConstantBuffers = g_dx12Device->getFrameConstantBuffers();
	DispatchDrawCallCpuDescriptorHeap& DrawDispatchCallCpuDescriptorHeap = g_dx12Device->getDispatchDrawCallCpuDescriptorHeap();

	ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
	ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();

	CachedRasterPsoDesc PSODesc;
	PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
	PSODesc.mLayout = nullptr;
	PSODesc.mBlendState = &getBlendState_AlphaBlending();
	PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
	PSODesc.mRasterizerState = &getRasterizerState_DefaultNoCulling();
	PSODesc.mRenderTargetCount = 1;
	PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
	PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PSODesc.mVS = FullScreenTriangleVertexShader;
	PSODesc.mPS = ColorDrawPixelShader;
	g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

	FrameConstantBuffers::FrameConstantBuffer CB = ConstantBuffers.AllocateFrameConstantBuffer(sizeof(ImageDrawConstantBuffer));
	ImageDrawConstantBuffer* CBData = (ImageDrawConstantBuffer*)CB.getCPUMemory();
	CBData->OutputWidthAndInv[0] = 1.0f;
	CBData->OutputWidthAndInv[1] = 1.0f / CBData->OutputWidthAndInv[0];
	CBData->OutputHeightAndInv[0] = 1.0f;
	CBData->OutputHeightAndInv[1] = 1.0f / CBData->OutputHeightAndInv[0];
	CBData->ImageBottomLeft[0] = 0;
	CBData->ImageBottomLeft[1] = 0;
	CBData->ImageSize[0] = 32768;
	CBData->ImageSize[1] = 32768;
	CBData->ColorAlpha[0] = v_blend[0];
	CBData->ColorAlpha[1] = v_blend[1];
	CBData->ColorAlpha[2] = v_blend[2];
	CBData->ColorAlpha[3] = v_blend[3];
	CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());

	IndexBufferSingleTri->resourceTransitionBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER);
	D3D12_INDEX_BUFFER_VIEW IndexBufferSingleTriBufferView = IndexBufferSingleTri->getIndexBufferView(DXGI_FORMAT_R32_UINT);
	CommandList->IASetIndexBuffer(&IndexBufferSingleTriBufferView);
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// set the primitive topology

	CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
}

// Render the world
// See https://fabiensanglard.net/quake2/quake2_opengl_renderer.php
void R_RenderView(void)
{
	SCOPED_GPU_TIMER(Quake2Frame, 100, 100, 100);

	ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
	ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();

	if (!MeshRenderBuffer)
	{
		MeshRenderBuffer = new RenderBufferGenericDynamic(1024 * 1024, D3D12_RESOURCE_FLAG_NONE);
	}

	// Set defaults graphic and compute root signatures
	CommandList->SetGraphicsRootSignature(g_dx12Device->GetDefaultGraphicRootSignature().getRootsignature());
	CommandList->SetComputeRootSignature(g_dx12Device->GetDefaultComputeRootSignature().getRootsignature());

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

	R_SetupFrame();

	R_SetFrustum();

	// Decompress the PVS and mark potentially Visible Polygons
	R_MarkLeaves();

	//TODO	R_DrawWorld();

	// Last render the sky
	SkyRender();

	{
		float* ptr = (float*)MeshRenderBuffer->Map();
		//*ptr++ = r_origin[0] + 100.0f * vpn[0];
		//*ptr++ = r_origin[1] + 100.0f * vpn[1];
		//*ptr++ = r_origin[2] + 100.0f * vpn[2];
		//*ptr++ = r_origin[0] - 100.0f * vpn[0] + 100.0f;
		//*ptr++ = r_origin[1] - 100.0f * vpn[1] + 100.0f;
		//*ptr++ = r_origin[2] - 100.0f * vpn[2] + 100.0f;
		//*ptr++ = r_origin[0] + 100.0f * vpn[0] + 100.0f * vup[0];
		//*ptr++ = r_origin[1] + 100.0f * vpn[1] + 100.0f * vup[1];
		//*ptr++ = r_origin[2] + 100.0f * vpn[2] + 100.0f * vup[2];
		*ptr = 1000.0f;
		ptr++;
		*ptr = 0.0f;
		ptr++;
		*ptr = 0.0f;
		ptr++;
		*ptr = 0.0f;
		ptr++;
		*ptr = 10000.0f;
		ptr++;
		*ptr = 0.0f;
		ptr++;
		*ptr = 0.0f;
		ptr++;
		*ptr = 0.0f;
		ptr++;
		*ptr = 1000.0f;
		ptr++;
		MeshRenderBuffer->UnmapAndUpload();

		//
		{
			ViewData vd = GetViewData();

			static InputLayout VtxLayout;
			if(VtxLayout.getLayoutDesc()->NumElements == 0)
				VtxLayout.appendSimpleVertexDataToInputLayout("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT);

			FrameConstantBuffers& ConstantBuffers = g_dx12Device->getFrameConstantBuffers();
			DispatchDrawCallCpuDescriptorHeap& DrawDispatchCallCpuDescriptorHeap = g_dx12Device->getDispatchDrawCallCpuDescriptorHeap();

			ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
			ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
			D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();

			CachedRasterPsoDesc PSODesc;
			PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
			PSODesc.mLayout = &VtxLayout;
			PSODesc.mBlendState = &getBlendState_Default();
			PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
			PSODesc.mRasterizerState = &getRasterizerState_DefaultNoCulling();
			PSODesc.mRenderTargetCount = 1;
			PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
			PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.mVS = MeshVertexShader;
			PSODesc.mPS = MeshDebugPixelShader;
			g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

			FrameConstantBuffers::FrameConstantBuffer CB = ConstantBuffers.AllocateFrameConstantBuffer(sizeof(MeshConstantBuffer));
			MeshConstantBuffer* CBData = (MeshConstantBuffer*)CB.getCPUMemory();
			CBData->MeshWorldMatrix = XMMatrixIdentity();
			CBData->ViewProjectionMatrix = vd.ViewProjectionMatrix;
			CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());

			IndexBufferSingleTri->resourceTransitionBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER);
			D3D12_INDEX_BUFFER_VIEW IndexBufferSingleTriBufferView = IndexBufferSingleTri->getIndexBufferView(DXGI_FORMAT_R32_UINT);
			CommandList->IASetIndexBuffer(&IndexBufferSingleTriBufferView);
			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// set the primitive topology

			MeshRenderBuffer->getRenderBuffer().resourceTransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			D3D12_VERTEX_BUFFER_VIEW VtxBufferView = MeshRenderBuffer->getRenderBuffer().getVertexBufferView(3 * sizeof(float));
			CommandList->IASetVertexBuffers(0, 1, &VtxBufferView);

			CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
		}
	}

	// Render entities on top of the world
	R_DrawEntitiesOnList();

	// Blend dynamic lights. Used instead of lightmap uppdate. 
	// ==> Unused. Does not look good.
	//R_RenderDlights();

//TODO	R_DrawParticles();

		// Alpha blend translucent surfaces
//TODO	R_DrawAlphaSurfaces();

		// Post effects (full screen red for damage, etc...)
	R_Flash();

	if (r_speeds->value)
	{
		ri.Con_Printf(PRINT_ALL, "%4i wpoly %4i epoly %i tex %i lmaps\n",
			c_brush_polys,
			c_alias_polys,
			c_visible_textures,
			c_visible_lightmaps);
	}

	R_SetLightLevel();
}

void R_ShutdownRenderView(void)
{
	if (MeshRenderBuffer)
	{
		delete MeshRenderBuffer;
	}
}

