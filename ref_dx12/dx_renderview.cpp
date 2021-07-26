
#include "dx_local.h"
#include "stdlib.h"


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


// speed up sin calculations - Ed
float	r_turbsin[] =
{
 0, 0.19633, 0.392541, 0.588517, 0.784137, 0.979285, 1.17384, 1.3677,
 1.56072, 1.75281, 1.94384, 2.1337, 2.32228, 2.50945, 2.69512, 2.87916,
 3.06147, 3.24193, 3.42044, 3.59689, 3.77117, 3.94319, 4.11282, 4.27998,
 4.44456, 4.60647, 4.76559, 4.92185, 5.07515, 5.22538, 5.37247, 5.51632,
 5.65685, 5.79398, 5.92761, 6.05767, 6.18408, 6.30677, 6.42566, 6.54068,
 6.65176, 6.75883, 6.86183, 6.9607, 7.05537, 7.14579, 7.23191, 7.31368,
 7.39104, 7.46394, 7.53235, 7.59623, 7.65552, 7.71021, 7.76025, 7.80562,
 7.84628, 7.88222, 7.91341, 7.93984, 7.96148, 7.97832, 7.99036, 7.99759,
 8, 7.99759, 7.99036, 7.97832, 7.96148, 7.93984, 7.91341, 7.88222,
 7.84628, 7.80562, 7.76025, 7.71021, 7.65552, 7.59623, 7.53235, 7.46394,
 7.39104, 7.31368, 7.23191, 7.14579, 7.05537, 6.9607, 6.86183, 6.75883,
 6.65176, 6.54068, 6.42566, 6.30677, 6.18408, 6.05767, 5.92761, 5.79398,
 5.65685, 5.51632, 5.37247, 5.22538, 5.07515, 4.92185, 4.76559, 4.60647,
 4.44456, 4.27998, 4.11282, 3.94319, 3.77117, 3.59689, 3.42044, 3.24193,
 3.06147, 2.87916, 2.69512, 2.50945, 2.32228, 2.1337, 1.94384, 1.75281,
 1.56072, 1.3677, 1.17384, 0.979285, 0.784137, 0.588517, 0.392541, 0.19633,
 9.79717e-16, -0.19633, -0.392541, -0.588517, -0.784137, -0.979285, -1.17384, -1.3677,
 -1.56072, -1.75281, -1.94384, -2.1337, -2.32228, -2.50945, -2.69512, -2.87916,
 -3.06147, -3.24193, -3.42044, -3.59689, -3.77117, -3.94319, -4.11282, -4.27998,
 -4.44456, -4.60647, -4.76559, -4.92185, -5.07515, -5.22538, -5.37247, -5.51632,
 -5.65685, -5.79398, -5.92761, -6.05767, -6.18408, -6.30677, -6.42566, -6.54068,
 -6.65176, -6.75883, -6.86183, -6.9607, -7.05537, -7.14579, -7.23191, -7.31368,
 -7.39104, -7.46394, -7.53235, -7.59623, -7.65552, -7.71021, -7.76025, -7.80562,
 -7.84628, -7.88222, -7.91341, -7.93984, -7.96148, -7.97832, -7.99036, -7.99759,
 -8, -7.99759, -7.99036, -7.97832, -7.96148, -7.93984, -7.91341, -7.88222,
 -7.84628, -7.80562, -7.76025, -7.71021, -7.65552, -7.59623, -7.53235, -7.46394,
 -7.39104, -7.31368, -7.23191, -7.14579, -7.05537, -6.9607, -6.86183, -6.75883,
 -6.65176, -6.54068, -6.42566, -6.30677, -6.18408, -6.05767, -5.92761, -5.79398,
 -5.65685, -5.51632, -5.37247, -5.22538, -5.07515, -4.92185, -4.76559, -4.60647,
 -4.44456, -4.27998, -4.11282, -3.94319, -3.77117, -3.59689, -3.42044, -3.24193,
 -3.06147, -2.87916, -2.69512, -2.50945, -2.32228, -2.1337, -1.94384, -1.75281,
 -1.56072, -1.3677, -1.17384, -0.979285, -0.784137, -0.588517, -0.392541, -0.19633,
};
#define TURBSCALE (256.0 / (2 * M_PI))


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////




static InputLayout MeshVertexFormatLayout;

MeshRenderer::MeshRenderer()
{
	bRecordingStarted = false;
	bCommandStarted = false;
	MeshVertexRenderBuffer = new RenderBufferGenericDynamic(VertexMemorySizeBytes, D3D12_RESOURCE_FLAG_NONE);
	MeshIndexRenderBuffer = new RenderBufferGenericDynamic(IndexMemorySizeBytes, D3D12_RESOURCE_FLAG_NONE);

	RenderCommands = new MeshRenderCommand[MaxCommandCount];
}

MeshRenderer::~MeshRenderer()
{
	delete MeshVertexRenderBuffer;
	delete MeshIndexRenderBuffer;
	free(RenderCommands);
}

void MeshRenderer::StartRecording()
{
	ATLASSERT(bRecordingStarted == false);
	ATLASSERT(bCommandStarted == false);

	AllocatedVertexBytes = 0;
	AllocatedIndexBytes = 0;
	RecordedVertexCount = 0;
	RecordedIndexCount = 0;

	RecordedRenderCommandCount = 0;
	CurrentCommand = nullptr;

	MeshVertexMemory = (MeshVertexFormat*)MeshVertexRenderBuffer->Map();
	MeshIndexMemory = (uint*)MeshIndexRenderBuffer->Map();

	bRecordingStarted = true;
}

void MeshRenderer::StopRecording()
{
	ATLASSERT(bRecordingStarted == true);
	ATLASSERT(bCommandStarted == false);

	MeshVertexRenderBuffer->UnmapAndUpload();
	MeshIndexRenderBuffer->UnmapAndUpload();

	MeshVertexRenderBuffer->getRenderBuffer().resourceTransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	MeshIndexRenderBuffer->getRenderBuffer().resourceTransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	bRecordingStarted = false;
}

void MeshRenderer::StartCommand(MeshRenderCommand::EType Type, float4x4 MeshWorldMatrix, D3D_PRIMITIVE_TOPOLOGY Topology)
{
	ATLASSERT(bRecordingStarted == true);
	ATLASSERT(bCommandStarted == false);

	CurrentCommand = &RenderCommands[RecordedRenderCommandCount++];
	bCommandStarted = true;

	//memset(CurrentCommand, 0, sizeof(MeshRenderCommand));

	CurrentCommand->Type = Type;
	CurrentCommand->Topology = Topology;

	XMStoreFloat4x4(&CurrentCommand->MeshWorldMatrix, MeshWorldMatrix);

	CurrentCommand->InstanceCount = 1;
	CurrentCommand->StartInstanceLocation = 0;

	if (Type == MeshRenderCommand::EType::DrawInstanced_Colored)
	{
		CurrentCommand->VertexCountPerInstance = 0;
		CurrentCommand->StartVertexLocation = RecordedVertexCount;
	}
	else
	{
		CurrentCommand->IndexCountPerInstance = 0;
		CurrentCommand->StartIndexLocation = RecordedIndexCount;
		CurrentCommand->BaseVertexLocation = RecordedVertexCount;
	}
}

void MeshRenderer::AppendVertex(MeshVertexFormat& NewVertex)
{
	ATLASSERT(bRecordingStarted == true);
	ATLASSERT(bCommandStarted == true);
	ATLASSERT((AllocatedVertexBytes + sizeof(MeshVertexFormat)) <= VertexMemorySizeBytes);

	// DrawInstanced
	RecordedVertexCount++;
	AllocatedVertexBytes += sizeof(MeshVertexFormat);

	*MeshVertexMemory = NewVertex;
	MeshVertexMemory++;

	if (CurrentCommand->Type == MeshRenderCommand::EType::DrawInstanced_Colored)
	{
		CurrentCommand->VertexCountPerInstance++;
	}
	else
	{
		// Nothing to do
	}
}

void MeshRenderer::EndCommand()
{
	ATLASSERT(bRecordingStarted == true);
	ATLASSERT(bCommandStarted == true);

	CurrentCommand = nullptr;
	bCommandStarted = false;
}

void MeshRenderer::ExecuteRenderCommands()
{
	ATLASSERT(bRecordingStarted == false);
	ATLASSERT(bCommandStarted == false);

	ViewData vd = GetViewData();

	FrameConstantBuffers& ConstantBuffers = g_dx12Device->getFrameConstantBuffers();
	DispatchDrawCallCpuDescriptorHeap& DrawDispatchCallCpuDescriptorHeap = g_dx12Device->getDispatchDrawCallCpuDescriptorHeap();

	ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
	ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();

	// Set defaults graphic and compute root signatures
	CommandList->SetGraphicsRootSignature(g_dx12Device->GetDefaultGraphicRootSignature().getRootsignature());
	CommandList->SetComputeRootSignature(g_dx12Device->GetDefaultComputeRootSignature().getRootsignature());

	CachedRasterPsoDesc PSODesc;
	PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
	PSODesc.mLayout = &MeshVertexFormatLayout;
	PSODesc.mBlendState = &getBlendState_Default();
	PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
	PSODesc.mRasterizerState = &getRasterizerState_DefaultNoCulling();
	PSODesc.mRenderTargetCount = 1;
	PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
	PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;


	MeshIndexRenderBuffer->getRenderBuffer().resourceTransitionBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER);
	D3D12_INDEX_BUFFER_VIEW MeshIndexRenderBufferView = MeshIndexRenderBuffer->getRenderBuffer().getIndexBufferView(DXGI_FORMAT_R32_UINT);
	CommandList->IASetIndexBuffer(&MeshIndexRenderBufferView);

	MeshVertexRenderBuffer->getRenderBuffer().resourceTransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	D3D12_VERTEX_BUFFER_VIEW MeshVertexRenderBufferView = MeshVertexRenderBuffer->getRenderBuffer().getVertexBufferView(sizeof(MeshVertexFormat));
	CommandList->IASetVertexBuffers(0, 1, &MeshVertexRenderBufferView);


	for (uint i = 0; i < RecordedRenderCommandCount; ++i)
	{
		MeshRenderCommand& Cmd = RenderCommands[i];

		switch (Cmd.Type)
		{
		case MeshRenderCommand::EType::DrawInstanced_Colored:
		{
			PSODesc.mVS = MeshVertexShader;
			PSODesc.mPS = MeshColorPixelShader;
			break;
		}
		default:
		{
			ATLASSERT(false);
			break;
		}
		}
			
		g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

		FrameConstantBuffers::FrameConstantBuffer CB = ConstantBuffers.AllocateFrameConstantBuffer(sizeof(MeshConstantBuffer));
		MeshConstantBuffer* CBData = (MeshConstantBuffer*)CB.getCPUMemory();

		CBData->MeshWorldMatrix = XMLoadFloat4x4(&Cmd.MeshWorldMatrix);
		CBData->ViewProjectionMatrix = vd.ViewProjectionMatrix;
		CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());

		CommandList->IASetPrimitiveTopology(Cmd.Topology);

		switch (Cmd.Type)
		{
		case MeshRenderCommand::EType::DrawInstanced_Colored:
		{
			CommandList->DrawInstanced(Cmd.VertexCountPerInstance, Cmd.InstanceCount, Cmd.StartVertexLocation, Cmd.StartInstanceLocation);
			break;
		}
		default:
		{
			ATLASSERT(false);
			break;
		}
		}
	}
		
}

MeshRenderer* gMeshRenderer = nullptr;


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////




void R_InitRenderView(void)
{
	MeshVertexFormatLayout.appendSimpleVertexDataToInputLayout("POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT);
	MeshVertexFormatLayout.appendSimpleVertexDataToInputLayout("TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT);
	MeshVertexFormatLayout.appendSimpleVertexDataToInputLayout("TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT);
	MeshVertexFormatLayout.appendSimpleVertexDataToInputLayout("TEXCOORD",	2, DXGI_FORMAT_R32G32B32A32_FLOAT);

	gMeshRenderer = new MeshRenderer();

	R_InitParticleRenderer();
}

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

	if (!r_drawentities->value)
		return;

		// draw non-transparent first
	for (i = 0; i < r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		if (currententity->flags & RF_TRANSLUCENT)
			continue;	// solid

		if (currententity->flags & RF_BEAM)
		{
			//R_DrawBeam(currententity);
			int i = 0;
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
				R_DrawBrushModel(currententity);
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
			int i = 0;
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


extern msurface_t	*r_alpha_surfaces;

void R_DrawAlphaSurfaces(void)
{
	msurface_t	*s;
	float		intens;

	//
	// go back to the world matrix
	//
//	qglLoadMatrixf(r_world_matrix);

//	qglEnable(GL_BLEND);
//	GL_TexEnv(GL_MODULATE);

	// the textures are prescaled up for a better lighting range,
	// so scale it back down
	intens = 1.0f;// gl_state.inverse_intensity;		// TODO?

	for (s = r_alpha_surfaces; s; s = s->texturechain)
	{
		c_brush_polys++;
		image_t* SurfaceTexture = s->texinfo->image;

		XMFLOAT4A Tmp;
		if (s->texinfo->flags & SURF_TRANS33)
			Tmp = XMFLOAT4A(intens, intens, intens, 0.33);
		else if (s->texinfo->flags & SURF_TRANS66)
			Tmp = XMFLOAT4A(intens, intens, intens, 0.66);
		else
			Tmp = XMFLOAT4A(intens, intens, intens, 1);
		float4 ColorAlpha = XMLoadFloat4A(&Tmp);

		if (s->flags & SURF_DRAWTURB)
		{
			//EmitWaterPolys(s);
			msurface_t *fa = s;

			glpoly_t	*p, *bp;
			float		*v;
			int			i;
			float		s, t, os, ot;
			float		scroll;
			float		rdt = r_newrefdef.time;

			if (fa->texinfo->flags & SURF_FLOWING)
				scroll = -64 * ((r_newrefdef.time*0.5) - (int)(r_newrefdef.time*0.5));
			else
				scroll = 0;


			for (bp = fa->polys; bp; bp = bp->next)
			{
				p = bp;


				gMeshRenderer->StartCommand(MeshRenderCommand::EType::DrawInstanced_Colored);

				MeshVertexFormat V0;
				bool bV0Set = false;
				MeshVertexFormat LastV;
				bool bLastVSet = false;

//				qglBegin(GL_TRIANGLE_FAN);
				for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
				{
					os = v[3];
					ot = v[4];

#if !id386
					s = os + r_turbsin[(int)((ot*0.125 + r_newrefdef.time) * TURBSCALE) & 255];
#else
					s = os + r_turbsin[Q_ftol(((ot*0.125 + rdt) * TURBSCALE)) & 255];
#endif
					s += scroll;
					s *= (1.0 / 64);

#if !id386
					t = ot + r_turbsin[(int)((os*0.125 + rdt) * TURBSCALE) & 255];
#else
					t = ot + r_turbsin[Q_ftol(((os*0.125 + rdt) * TURBSCALE)) & 255];
#endif
					t *= (1.0 / 64);

					MeshVertexFormat Vertex;
					memcpy(Vertex.Position, v, sizeof(Vertex.Position));
					Vertex.ColorAlpha[0] = Tmp.x;
					Vertex.ColorAlpha[1] = Tmp.y;
					Vertex.ColorAlpha[2] = Tmp.z;
					Vertex.ColorAlpha[3] = Tmp.w;

					if (bLastVSet)
					{
						gMeshRenderer->AppendVertex(V0);
						gMeshRenderer->AppendVertex(LastV);
						gMeshRenderer->AppendVertex(Vertex);
					}

//					qglTexCoord2f(s, t);
//					qglVertex3fv(v);

					if (bV0Set)
					{
						LastV = Vertex;
						bLastVSet = true;
					}
					if (!bV0Set)
					{
						V0 = Vertex;
						bV0Set = true;
					}
				}
//				qglEnd();

				gMeshRenderer->EndCommand();
			}
		}
		else if (s->texinfo->flags & SURF_FLOWING)			// PGM	9/16/98
		{
			int i = 0;
			//DrawGLFlowingPoly(s);							// PGM
			// TODO

		}
		else
		{
			int i = 0;
			//DrawGLPoly(s->polys);
			// TODO
		}
	}

//	GL_TexEnv(GL_REPLACE);
//	qglColor4f(1, 1, 1, 1);
//	qglDisable(GL_BLEND);

	r_alpha_surfaces = NULL;
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

	gMeshRenderer->StartRecording();

	// Mark polygon affected by dynamic light
	R_PushDlights();

	R_SetupFrame();

	R_SetFrustum();

	// Decompress the PVS and mark potentially Visible Polygons
	R_MarkLeaves();

//TODO	R_DrawWorld();

	// Last render the sky
	SkyRender();

	/*gMeshRenderer->StartCommand(MeshRenderCommand::EType::DrawInstanced_Colored);
	MeshVertexFormat v;
	v.Position[0] = 1000.0f;
	v.Position[1] = 0;
	v.Position[2] = 0;
	v.SurfaceUV[0] = 0;
	v.SurfaceUV[1] = 0;
	v.LightmapUV[0] = 0;
	v.LightmapUV[1] = 0;
	v.ColorAlpha[0] = 1;
	v.ColorAlpha[1] = 0;
	v.ColorAlpha[2] = 0;
	v.ColorAlpha[3] = 1;
	gMeshRenderer->AppendVertex(v);
	v.Position[0] = 0;
	v.Position[1] = 1000.0f;
	v.Position[2] = 0;
	v.SurfaceUV[0] = 0;
	v.SurfaceUV[1] = 0;
	v.LightmapUV[0] = 0;
	v.LightmapUV[1] = 0;
	v.ColorAlpha[0] = 0;
	v.ColorAlpha[1] = 1;
	v.ColorAlpha[2] = 0;
	v.ColorAlpha[3] = 1;
	gMeshRenderer->AppendVertex(v);
	v.Position[0] = 0;
	v.Position[1] = 0;
	v.Position[2] = 1000.0f;
	v.SurfaceUV[0] = 0;
	v.SurfaceUV[1] = 0;
	v.LightmapUV[0] = 0;
	v.LightmapUV[1] = 0;
	v.ColorAlpha[0] = 0;
	v.ColorAlpha[1] = 0;
	v.ColorAlpha[2] = 1;
	v.ColorAlpha[3] = 1;
	gMeshRenderer->AppendVertex(v);
	gMeshRenderer->EndCommand();

	gMeshRenderer->StartCommand(MeshRenderCommand::EType::DrawInstanced_Colored);
	v.Position[0] = -1000.0f;
	v.Position[1] = 0;
	v.Position[2] = 0;
	v.SurfaceUV[0] = 0;
	v.SurfaceUV[1] = 0;
	v.LightmapUV[0] = 0;
	v.LightmapUV[1] = 0;
	v.ColorAlpha[0] = 1;
	v.ColorAlpha[1] = 0;
	v.ColorAlpha[2] = 0;
	v.ColorAlpha[3] = 1;
	gMeshRenderer->AppendVertex(v);
	v.Position[0] = 0;
	v.Position[1] = -1000.0f;
	v.Position[2] = 0;
	v.SurfaceUV[0] = 0;
	v.SurfaceUV[1] = 0;
	v.LightmapUV[0] = 0;
	v.LightmapUV[1] = 0;
	v.ColorAlpha[0] = 1;
	v.ColorAlpha[1] = 1;
	v.ColorAlpha[2] = 0;
	v.ColorAlpha[3] = 1;
	gMeshRenderer->AppendVertex(v);
	v.Position[0] = 0;
	v.Position[1] = 0;
	v.Position[2] = -1000.0f;
	v.SurfaceUV[0] = 0;
	v.SurfaceUV[1] = 0;
	v.LightmapUV[0] = 0;
	v.LightmapUV[1] = 0;
	v.ColorAlpha[0] = 1;
	v.ColorAlpha[1] = 0;
	v.ColorAlpha[2] = 1;
	v.ColorAlpha[3] = 1;
	gMeshRenderer->AppendVertex(v);
	gMeshRenderer->EndCommand();*/


	// Render entities on top of the world
	R_DrawEntitiesOnList();

	// Blend dynamic lights. Can be used instead of lightmap uppdate. 
	// ==> Not implemented. Does not look good.
	//R_RenderDlights();

	R_RenderParticles();

	// Alpha blend translucent surfaces
//	R_DrawAlphaSurfaces();

	gMeshRenderer->StopRecording();
	gMeshRenderer->ExecuteRenderCommands();

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
	R_ShutdownParticleRenderer();

	if (gMeshRenderer)
	{
		delete gMeshRenderer;
		gMeshRenderer = nullptr;
	}
}

