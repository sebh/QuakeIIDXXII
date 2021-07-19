
#include "dx_local.h"

#include <vector>

std::vector<DrawImageCall> DrawImageCalls;


void DrawBeginFrame()
{
	DrawImageCalls.clear();
	DrawImageCalls.reserve(512);
}

void AddDrawImage(DrawImageCall& DrawImageCall)
{
	DrawImageCalls.push_back(DrawImageCall);
}

void DrawAllImages(unsigned int BackBufferWidth, unsigned int BackBufferHeight)
{
	SCOPED_GPU_TIMER(DrawAllImages, 100, 100, 100);

	ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
	ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();
	float AspectRatioXOverY = float(BackBuffer->GetDesc().Width) / float(BackBuffer->GetDesc().Height);

	// Set defaults graphic and compute root signatures
	CommandList->SetGraphicsRootSignature(g_dx12Device->GetDefaultGraphicRootSignature().getRootsignature());
	CommandList->SetComputeRootSignature(g_dx12Device->GetDefaultComputeRootSignature().getRootsignature());

	// Set the common descriptor heap
	std::vector<ID3D12DescriptorHeap*> descriptorHeaps;
	descriptorHeaps.push_back(g_dx12Device->getFrameDispatchDrawCallGpuDescriptorHeap()->getHeap());
	CommandList->SetDescriptorHeaps(uint(descriptorHeaps.size()), descriptorHeaps.data());

	// Set the Viewport
	D3D12_VIEWPORT Viewport;
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.Width = BackBufferWidth;
	Viewport.Height = BackBufferHeight;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	CommandList->RSSetViewports(1, &Viewport);
	D3D12_RECT ScissorRect;
	ScissorRect.left = 0;
	ScissorRect.top = 0;
	ScissorRect.right = BackBufferWidth;
	ScissorRect.bottom = BackBufferHeight;
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Some rendering tests
	{
		// Set PSO and render targets
		CachedRasterPsoDesc PSODesc;
		PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
		PSODesc.mLayout = nullptr;
		PSODesc.mVS = TestVertexShader;
		PSODesc.mPS = TestPixelShader;
		PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
		PSODesc.mRasterizerState = &getRasterizerState_Default();
		PSODesc.mBlendState = &getBlendState_Default();
		PSODesc.mRenderTargetCount = 1;
		PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
		PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

		// Set other raster properties
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// set the primitive topology

		IndexBuffer->resourceTransitionBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER);
		D3D12_INDEX_BUFFER_VIEW IndexBufferView = IndexBuffer->getIndexBufferView(DXGI_FORMAT_R32_UINT);
		CommandList->IASetIndexBuffer(&IndexBufferView);

		CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
	}
}


