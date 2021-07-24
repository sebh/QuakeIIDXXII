
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

	IndexBufferQuadTris->resourceTransitionBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER);
	D3D12_INDEX_BUFFER_VIEW QuadIndexBufferView = IndexBufferQuadTris->getIndexBufferView(DXGI_FORMAT_R32_UINT);

	// Some rendering tests
#if 0
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

		CommandList->IASetIndexBuffer(&QuadIndexBufferView);

		CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
#endif

	// Draw the 2d images
	// TODO one draw call for all the same types instead of per element
	for (auto& dic : DrawImageCalls)
	{
		CachedRasterPsoDesc PSODesc;
		PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
		PSODesc.mLayout = nullptr;
		PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
		PSODesc.mRasterizerState = &getRasterizerState_DefaultNoCulling();
		PSODesc.mRenderTargetCount = 1;
		PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
		PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

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
			PSODesc.mVS = ImageDrawVertexShader;
			PSODesc.mPS = ImageDrawPixelShader;

			PSODesc.mBlendState = dic.Image && dic.Image->has_alpha ? &getBlendState_PremultipledAlpha() : &getBlendState_Default();

			CBData->ImageBottomLeft[0] = dic.x;
			CBData->ImageBottomLeft[1] = dic.y;
			CBData->ImageSize[0] = dic.w;
			CBData->ImageSize[1] = dic.h;

			DispatchDrawCallCpuDescriptorHeap::Call CallDescriptors = DrawDispatchCallCpuDescriptorHeap.AllocateCall(g_dx12Device->GetDefaultGraphicRootSignature());
			CallDescriptors.SetSRV(0, *dic.Image->RenderTexture);
			CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex_DescriptorTable0, CallDescriptors.getRootDescriptorTableGpuHandle());
			break;
		}
		case DrawImageCallType::Draw_Tex:
		{
			PSODesc.mVS = ImageDrawVertexShader;
			PSODesc.mPS = ImageDrawPixelShader;

			PSODesc.mBlendState = &getBlendState_Default();

			CBData->ImageBottomLeft[0] = dic.x;
			CBData->ImageBottomLeft[1] = dic.y;
			CBData->ImageSize[0] = dic.w;
			CBData->ImageSize[1] = dic.h;

			DispatchDrawCallCpuDescriptorHeap::Call CallDescriptors = DrawDispatchCallCpuDescriptorHeap.AllocateCall(g_dx12Device->GetDefaultGraphicRootSignature());
			CallDescriptors.SetSRV(0, *dic.Texture);
			CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex_DescriptorTable0, CallDescriptors.getRootDescriptorTableGpuHandle());
			break;
		}
		case DrawImageCallType::Draw_Fill:
		{
			PSODesc.mVS = ImageDrawVertexShader;
			PSODesc.mPS = ColorDrawPixelShader;

			PSODesc.mBlendState = &getBlendState_Default();

			CBData->ImageBottomLeft[0] = dic.x;
			CBData->ImageBottomLeft[1] = dic.y;
			CBData->ImageSize[0] = dic.w;
			CBData->ImageSize[1] = dic.h;

			union
			{
				unsigned	c;
				byte		v[4];
			} color;

			if ((unsigned)dic.c > 255)
			{
				ErrorExit("Draw_Fill: bad color\n");
			}

			color.c = d_8to24table[dic.c];

			CBData->ColorAlpha[0] = color.v[0] / 255.0f;
			CBData->ColorAlpha[1] = color.v[1] / 255.0f;
			CBData->ColorAlpha[2] = color.v[2] / 255.0f;
			CBData->ColorAlpha[3] = 1.0f;
			break;
		}
		case DrawImageCallType::Draw_Char:
		{
			int				row, col;
			float			frow, fcol, size;

			int num = dic.c & 255;

			if ((num & 127) == 32)
				continue;		// space

			if (dic.y <= -8)
				continue;		// totally off screen

			row = num >> 4;
			col = num & 15;

			frow = row * 0.0625;
			fcol = col * 0.0625;
			size = 0.0625;

			PSODesc.mVS = ImageDrawVertexShader;
			PSODesc.mPS = CharDrawPixelShader;

			PSODesc.mBlendState = &getBlendState_PremultipledAlpha();

			CBData->ImageBottomLeft[0] = dic.x;
			CBData->ImageBottomLeft[1] = dic.y;
			CBData->ImageSize[0] = 8;
			CBData->ImageSize[1] = 8;

			// Reusing color input for uv data assuming font is always white
			CBData->ColorAlpha[0] = fcol;
			CBData->ColorAlpha[1] = frow;
			CBData->ColorAlpha[2] = size;
			CBData->ColorAlpha[3] = size;

			DispatchDrawCallCpuDescriptorHeap::Call CallDescriptors = DrawDispatchCallCpuDescriptorHeap.AllocateCall(g_dx12Device->GetDefaultGraphicRootSignature());
			CallDescriptors.SetSRV(0, *r_charstexture->RenderTexture);
			CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex_DescriptorTable0, CallDescriptors.getRootDescriptorTableGpuHandle());
			break;
		}
		case DrawImageCallType::Draw_TileClear:
		{
			PSODesc.mVS = ImageDrawVertexShader;
			PSODesc.mPS = TiledImageDrawPixelShader;

			PSODesc.mBlendState = &getBlendState_Default();

			CBData->ImageBottomLeft[0] = dic.x;
			CBData->ImageBottomLeft[1] = dic.y;
			CBData->ImageSize[0] = dic.w;
			CBData->ImageSize[1] = dic.h;

			DispatchDrawCallCpuDescriptorHeap::Call CallDescriptors = DrawDispatchCallCpuDescriptorHeap.AllocateCall(g_dx12Device->GetDefaultGraphicRootSignature());
			CallDescriptors.SetSRV(0, *dic.Image->RenderTexture);
			CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex_DescriptorTable0, CallDescriptors.getRootDescriptorTableGpuHandle());
			break;
		}
		case DrawImageCallType::Draw_FadeScreen:
		{
			PSODesc.mVS = ImageDrawVertexShader;
			PSODesc.mPS = ColorDrawPixelShader;

			PSODesc.mBlendState = &getBlendState_PremultipledAlpha();

			CBData->ImageBottomLeft[0] = 0;
			CBData->ImageBottomLeft[1] = 0;
			CBData->ImageSize[0] = 32768;
			CBData->ImageSize[1] = 32768;

			CBData->ColorAlpha[0] = 0.0f;
			CBData->ColorAlpha[1] = 0.0f;
			CBData->ColorAlpha[2] = 0.0f;
			CBData->ColorAlpha[3] = 0.8f;
			break;
		}
		default:
			ErrorExit("DrawAllImages : unkown DrawImageCallType\n");
			break;
		}
		
		g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// set the primitive topology
		CommandList->IASetIndexBuffer(&QuadIndexBufferView);

		CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());

		CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
}


