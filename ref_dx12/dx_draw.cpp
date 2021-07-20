
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

	FrameConstantBuffers& ConstantBuffers = g_dx12Device->getFrameConstantBuffers();
	DispatchDrawCallCpuDescriptorHeap& DrawDispatchCallCpuDescriptorHeap = g_dx12Device->getDispatchDrawCallCpuDescriptorHeap();

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

	IndexBuffer->resourceTransitionBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER);
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = IndexBuffer->getIndexBufferView(DXGI_FORMAT_R32_UINT);

	// Some rendering tests
	{
		// Set PSO and render targets
		CachedRasterPsoDesc PSODesc;
		PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
		PSODesc.mLayout = nullptr;
		PSODesc.mVS = TestVertexShader;
		PSODesc.mPS = TestPixelShader;
		PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
		PSODesc.mRasterizerState = &getRasterizerState_DefaultNoCulling();
		PSODesc.mBlendState = &getBlendState_Default();
		PSODesc.mRenderTargetCount = 1;
		PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
		PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

		// Set other raster properties
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// set the primitive topology

		CommandList->IASetIndexBuffer(&IndexBufferView);

		CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
	}

	// Draw the 2d images
	for (auto& dic : DrawImageCalls)
	{
		CachedRasterPsoDesc PSODesc;
		PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
		PSODesc.mLayout = nullptr;
		PSODesc.mVS = ImageDrawVertexShader;
		PSODesc.mPS = ImageDrawPixelShader;
		PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
		PSODesc.mRasterizerState = &getRasterizerState_Default();
		PSODesc.mBlendState = &getBlendState_Default();
		PSODesc.mRenderTargetCount = 1;
		PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
		PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

		// Set other raster properties
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// set the primitive topology

		CommandList->IASetIndexBuffer(&IndexBufferView);


		FrameConstantBuffers::FrameConstantBuffer CB = ConstantBuffers.AllocateFrameConstantBuffer(sizeof(ImageDrawConstantBuffer));
		ImageDrawConstantBuffer* CBData = (ImageDrawConstantBuffer*)CB.getCPUMemory();
		CBData->OutputWidthAndInv[0] = float(BackBufferWidth);
		CBData->OutputWidthAndInv[1] = 1.0f / CBData->OutputWidthAndInv[0];
		CBData->OutputHeightAndInv[0] = float(BackBufferHeight);
		CBData->OutputHeightAndInv[1] = 1.0f / CBData->OutputHeightAndInv[0];

		switch (dic.Type)
		{
		case DrawImageCallType::Draw_Pic:
		{
			CBData->ImageBottomLeft[0] = dic.DrawPic.x;
			CBData->ImageBottomLeft[1] = dic.DrawPic.y;
			CBData->ImageSize[0] = dic.Image->width;
			CBData->ImageSize[1] = dic.Image->height;

			DispatchDrawCallCpuDescriptorHeap::Call CallDescriptors = DrawDispatchCallCpuDescriptorHeap.AllocateCall(g_dx12Device->GetDefaultGraphicRootSignature());
			CallDescriptors.SetSRV(0, *dic.Image->RenderTexture);
			CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex_DescriptorTable0, CallDescriptors.getRootDescriptorTableGpuHandle());
			break;
		}
		default:
			break;
		}

		CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());


		CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
	}
}


