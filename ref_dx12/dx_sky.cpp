
#include "dx_local.h"


static char		skyname[MAX_QPATH];
static float	skyrotate;
static vec3_t	skyaxis;
static image_t*	sky_images[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

static char*	suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };


void SkyRegisterTexture(char* name, float rotate, vec3_t axis)
{
	int		i;
	char	pathname[MAX_QPATH];

	strncpy(skyname, name, sizeof(skyname) - 1);
	skyrotate = rotate;
	VectorCopy(axis, skyaxis);

	for (i = 0; i < 6; i++)
	{
		Com_sprintf(pathname, sizeof(pathname), "env/%s%s.tga", skyname, suf[i]);

		sky_images[i] = FindImage(pathname, it_sky);
		if (!sky_images[i])
			sky_images[i] = r_notexture;
	}
}

void SkyUnregisterTexture()
{
	sky_images[0] = sky_images[1] = sky_images[2] = sky_images[3] = sky_images[4] = sky_images[5] = nullptr;
}

void SkyRender()
{
	if (sky_images[0])
	{
		FrameConstantBuffers& ConstantBuffers = g_dx12Device->getFrameConstantBuffers();
		DispatchDrawCallCpuDescriptorHeap& DrawDispatchCallCpuDescriptorHeap = g_dx12Device->getDispatchDrawCallCpuDescriptorHeap();

		ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
		ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
		D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();

		IndexBufferQuadTris->resourceTransitionBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER);
		D3D12_INDEX_BUFFER_VIEW QuadIndexBufferView = IndexBufferQuadTris->getIndexBufferView(DXGI_FORMAT_R32_UINT);

		// Set defaults graphic and compute root signatures
		CommandList->SetGraphicsRootSignature(g_dx12Device->GetDefaultGraphicRootSignature().getRootsignature());
		CommandList->SetComputeRootSignature(g_dx12Device->GetDefaultComputeRootSignature().getRootsignature());

		// Set the common descriptor heap
		std::vector<ID3D12DescriptorHeap*> descriptorHeaps;
		descriptorHeaps.push_back(g_dx12Device->getFrameDispatchDrawCallGpuDescriptorHeap()->getHeap());
		CommandList->SetDescriptorHeaps(uint(descriptorHeaps.size()), descriptorHeaps.data());

		ViewData vd = GetViewData();
		for (int i = 0; i < 6; i++)
		{
			CachedRasterPsoDesc PSODesc;
			PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
			PSODesc.mVS = SkyVertexShader;
			PSODesc.mPS = SkyPixelShader;
			PSODesc.mLayout = nullptr;
			PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
			PSODesc.mRasterizerState = &getRasterizerState_DefaultNoCulling();
			PSODesc.mBlendState = &getBlendState_Default();
			PSODesc.mRenderTargetCount = 1;
			PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
			PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

			FrameConstantBuffers::FrameConstantBuffer CB = ConstantBuffers.AllocateFrameConstantBuffer(sizeof(SkyConstantBuffer));
			SkyConstantBuffer* CBData = (SkyConstantBuffer*)CB.getCPUMemory();
			CBData->ViewProjectionMatrix = vd.ViewProjectionMatrix;
			CBData->Face = i;

			DispatchDrawCallCpuDescriptorHeap::Call CallDescriptors = DrawDispatchCallCpuDescriptorHeap.AllocateCall(g_dx12Device->GetDefaultGraphicRootSignature());
			CallDescriptors.SetSRV(0, *sky_images[i]->RenderTexture);
			CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex_DescriptorTable0, CallDescriptors.getRootDescriptorTableGpuHandle());

			CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());

			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// set the primitive topology
			CommandList->IASetIndexBuffer(&QuadIndexBufferView);

			CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		}
	}
}


