
#include "dx_local.h"



struct ImageVertexFormat
{
	float Position[2];
	float SurfaceUV[2];
	float ColorAlpha[4];
};


static InputLayout ImageVertexFormatLayout;

struct ImageRenderCommand
{
	DrawImageCallType Type;
	bool bPreMultBlendingEnabled;

	image_t* Image;					// If Image is not null, use it...
	RenderTexture* SurfaceTexture;	// ... Otherwise use SurfaceTexture if not null

	// Used by both DrawInstanced
	UINT InstanceCount;
	UINT StartInstanceLocation;
	UINT VertexCountPerInstance;
	UINT StartVertexLocation;
};

#define ImageVertexMemorySizeBytes	4 * 1024 * 1024
#define ImageMaxCommandCount		1024

class ImageRenderer
{
public:

	ImageRenderer()
	{
		ImageVertexRenderBuffer = new RenderBufferGenericDynamic(ImageVertexMemorySizeBytes, D3D12_RESOURCE_FLAG_NONE);

		RenderCommands = new ImageRenderCommand[ImageMaxCommandCount];
	}

	~ImageRenderer()
	{
		delete ImageVertexRenderBuffer;
		delete RenderCommands;
	}

	void StartRecording()
	{
		ATLASSERT(bRecordingStarted == false);

		AllocatedVertexBytes = 0;
		AllocatedIndexBytes = 0;
		RecordedVertexCount = 0;
		RecordedIndexCount = 0;

		RecordedRenderCommandCount = 0;
		CurrentCommand = nullptr;

		ImageVertexMemory = (ImageVertexFormat*)ImageVertexRenderBuffer->Map();

		bRecordingStarted = true;
	}

#define D_BATCH_IMAGE 1

	void ImageDraw(DrawImageCall& dic)
	{
		ATLASSERT(bRecordingStarted == true);
		ATLASSERT(RecordedRenderCommandCount < ImageMaxCommandCount);

		bool bPreMultBlendingEnabled = false;
		image_t* Image = nullptr;
		RenderTexture* SurfaceTexture = nullptr;
		switch (dic.Type)
		{
		case DrawImageCallType::Draw_Pic:
		{
			if (dic.Image)
			{
				Image = dic.Image;
				bPreMultBlendingEnabled = dic.Image->has_alpha;
			}
			break;
		}
		case DrawImageCallType::Draw_Tex:
		{
			SurfaceTexture = dic.Texture;
			break;
		}
		case DrawImageCallType::Draw_Fill:
		{
			break;
		}
		case DrawImageCallType::Draw_Char:
		{
			bPreMultBlendingEnabled = true;
			SurfaceTexture = r_charstexture->RenderTexture;
			break;
		}
		case DrawImageCallType::Draw_TileClear:
		{
			SurfaceTexture = dic.Image->RenderTexture;
			break;
		}
		case DrawImageCallType::Draw_FadeScreen:
		{
			bPreMultBlendingEnabled = true;
			break;
		}
		default:
			ErrorExit("DrawAllImages : unkown DrawImageCallType\n");
			break;
		}

#if D_BATCH_IMAGE
		if (CurrentCommand != nullptr
			&& CurrentCommand->Type == dic.Type
			&& CurrentCommand->Image == Image
			&& CurrentCommand->SurfaceTexture == SurfaceTexture
			&& CurrentCommand->bPreMultBlendingEnabled == bPreMultBlendingEnabled)
		{
			// Fall through, re-use the same command as the last one to insert triangle in the same batch
			int i = 0;
		}
		else
#endif
		{
			CurrentCommand = &RenderCommands[RecordedRenderCommandCount++];

			CurrentCommand->Type = dic.Type;

			CurrentCommand->Image = Image;
			CurrentCommand->SurfaceTexture = SurfaceTexture;

			CurrentCommand->bPreMultBlendingEnabled = bPreMultBlendingEnabled;

			CurrentCommand->InstanceCount = 1;
			CurrentCommand->StartInstanceLocation = 0;
			CurrentCommand->VertexCountPerInstance = 0;
			CurrentCommand->StartVertexLocation = RecordedVertexCount;
		}

		auto AppendVertex = [&](ImageVertexFormat& NewVertex)
		{
			ATLASSERT(bRecordingStarted == true);
			ATLASSERT((AllocatedVertexBytes + sizeof(ImageVertexFormat)) <= VertexMemorySizeBytes);

			// DrawInstanced
			RecordedVertexCount++;
			AllocatedVertexBytes += sizeof(ImageVertexFormat);

			*ImageVertexMemory = NewVertex;
			ImageVertexMemory++;

			CurrentCommand->VertexCountPerInstance++;
		};

		ImageVertexFormat v0, v1, v2, v3;

		auto SetQuadUniformColor = [&](const float* RGBA)
		{
			memcpy(v0.ColorAlpha, RGBA, sizeof(float) * 4);
			memcpy(v1.ColorAlpha, RGBA, sizeof(float) * 4);
			memcpy(v2.ColorAlpha, RGBA, sizeof(float) * 4);
			memcpy(v3.ColorAlpha, RGBA, sizeof(float) * 4);
		};

		auto AppendQuad = [&]()
		{
			AppendVertex(v0);
			AppendVertex(v1);
			AppendVertex(v2);
			AppendVertex(v0);
			AppendVertex(v2);
			AppendVertex(v3);
		};

		const float DefaultQuadColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		switch (dic.Type)
		{
		case DrawImageCallType::Draw_Pic:
		case DrawImageCallType::Draw_Tex:
		case DrawImageCallType::Draw_TileClear:
		{
			v0.Position[0] = dic.x;
			v0.Position[1] = dic.y;
			v0.SurfaceUV[0] = 0;
			v0.SurfaceUV[1] = 0;
			v1.Position[0] = dic.x + dic.w;
			v1.Position[1] = dic.y;
			v1.SurfaceUV[0] = 1;
			v1.SurfaceUV[1] = 0;
			v2.Position[0] = dic.x + dic.w;
			v2.Position[1] = dic.y + dic.h;
			v2.SurfaceUV[0] = 1;
			v2.SurfaceUV[1] = 1;
			v3.Position[0] = dic.x;
			v3.Position[1] = dic.y + dic.h;
			v3.SurfaceUV[0] = 0;
			v3.SurfaceUV[1] = 1;

			SetQuadUniformColor(DefaultQuadColor);
			AppendQuad();
			break;
		}
		case DrawImageCallType::Draw_Fill:
		{
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
			v0.ColorAlpha[0] = color.v[0] / 255.0f;
			v0.ColorAlpha[1] = color.v[1] / 255.0f;
			v0.ColorAlpha[2] = color.v[2] / 255.0f;
			v0.ColorAlpha[3] = 1.0f;
			SetQuadUniformColor(v0.ColorAlpha);

			v0.Position[0] = dic.x;
			v0.Position[1] = dic.y;
			v0.SurfaceUV[0] = 0;
			v0.SurfaceUV[1] = 0;
			v1.Position[0] = dic.x + dic.w;
			v1.Position[1] = dic.y;
			v1.SurfaceUV[0] = 1;
			v1.SurfaceUV[1] = 0;
			v2.Position[0] = dic.x + dic.w;
			v2.Position[1] = dic.y + dic.h;
			v2.SurfaceUV[0] = 1;
			v2.SurfaceUV[1] = 1;
			v3.Position[0] = dic.x;
			v3.Position[1] = dic.y + dic.h;
			v3.SurfaceUV[0] = 0;
			v3.SurfaceUV[1] = 1;
			AppendQuad();
			break;
		}
		case DrawImageCallType::Draw_Char:
		{
			int num = dic.c & 255;
			if ((num & 127) != 32 && (dic.y > -8))
			{
				int row = num >> 4;
				int col = num & 15;

				float frow = row * 0.0625;
				float fcol = col * 0.0625;
				float size = 0.0625;

				v0.Position[0] = dic.x;
				v0.Position[1] = dic.y;
				v0.SurfaceUV[0] = fcol;
				v0.SurfaceUV[1] = frow;
				v1.Position[0] = dic.x + 8;
				v1.Position[1] = dic.y;
				v1.SurfaceUV[0] = fcol + size;
				v1.SurfaceUV[1] = frow;
				v2.Position[0] = dic.x + 8;
				v2.Position[1] = dic.y + 8;
				v2.SurfaceUV[0] = fcol + size;
				v2.SurfaceUV[1] = frow + size;
				v3.Position[0] = dic.x;
				v3.Position[1] = dic.y + 8;
				v3.SurfaceUV[0] = fcol;
				v3.SurfaceUV[1] = frow + size;

				SetQuadUniformColor(DefaultQuadColor);
				AppendQuad();
			}

			break;
		}
		case DrawImageCallType::Draw_FadeScreen:
		{
			float FadeColorAlpha[4] = { 0.0f, 0.0f, 0.0f, 0.8f };
			SetQuadUniformColor(FadeColorAlpha);

			float size = 32678.0f;
			v0.Position[0] = dic.x;
			v0.Position[1] = dic.y;
			v0.SurfaceUV[0] = 0;
			v0.SurfaceUV[1] = 0;
			v1.Position[0] = dic.x + size;
			v1.Position[1] = dic.y;
			v1.SurfaceUV[0] = 1;
			v1.SurfaceUV[1] = 0;
			v2.Position[0] = dic.x + size;
			v2.Position[1] = dic.y + size;
			v2.SurfaceUV[0] = 1;
			v2.SurfaceUV[1] = 1;
			v3.Position[0] = dic.x;
			v3.Position[1] = dic.y + size;
			v3.SurfaceUV[0] = 0;
			v3.SurfaceUV[1] = 1;
			AppendQuad();
			break;
		}
		default:
			ErrorExit("DrawAllImages : unkown DrawImageCallType\n");
			break;
		}
	}

	void StopRecording()
	{
		ATLASSERT(bRecordingStarted == true);

		ImageVertexRenderBuffer->UnmapAndUpload();

		ImageVertexRenderBuffer->getRenderBuffer().resourceTransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		bRecordingStarted = false;
	}

	void ExecuteRenderCommands(unsigned int BackBufferWidth, unsigned int BackBufferHeight)
	{
		ATLASSERT(bRecordingStarted == false);

		FrameConstantBuffers& ConstantBuffers = g_dx12Device->getFrameConstantBuffers();
		DispatchDrawCallCpuDescriptorHeap& DrawDispatchCallCpuDescriptorHeap = g_dx12Device->getDispatchDrawCallCpuDescriptorHeap();

		ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
		ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
		D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();

		// Set defaults graphic and compute root signatures
		CommandList->SetGraphicsRootSignature(g_dx12Device->GetDefaultGraphicRootSignature().getRootsignature());
		CommandList->SetComputeRootSignature(g_dx12Device->GetDefaultComputeRootSignature().getRootsignature());

		// Set the common descriptor heap
		std::vector<ID3D12DescriptorHeap*> descriptorHeaps;
		descriptorHeaps.push_back(g_dx12Device->getFrameDispatchDrawCallGpuDescriptorHeap()->getHeap());
		CommandList->SetDescriptorHeaps(uint(descriptorHeaps.size()), descriptorHeaps.data());

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

		CachedRasterPsoDesc PSODesc;
		PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
		PSODesc.mLayout = &ImageVertexFormatLayout;
		PSODesc.mDepthStencilState = &getDepthStencilState_Disabled();
		PSODesc.mRasterizerState = &getRasterizerState_DefaultNoCulling();
		PSODesc.mRenderTargetCount = 1;
		PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
		PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		ImageVertexRenderBuffer->getRenderBuffer().resourceTransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		D3D12_VERTEX_BUFFER_VIEW ImageVertexRenderBufferView = ImageVertexRenderBuffer->getRenderBuffer().getVertexBufferView(sizeof(ImageVertexFormat));
		CommandList->IASetVertexBuffers(0, 1, &ImageVertexRenderBufferView);

		for (uint i = 0; i < RecordedRenderCommandCount; ++i)
		{
			ImageRenderCommand& Cmd = RenderCommands[i];

			// Enable blend state if needed
			PSODesc.mBlendState = Cmd.bPreMultBlendingEnabled ? &getBlendState_PremultipledAlpha() : &getBlendState_Default();

			PSODesc.mVS = ColoredImageVertexShader;
			PSODesc.mPS = ColoredImagePixelShader;

			DispatchDrawCallCpuDescriptorHeap::Call CallDescriptors = DrawDispatchCallCpuDescriptorHeap.AllocateCall(g_dx12Device->GetDefaultGraphicRootSignature());
			if (Cmd.Image)
			{
				CallDescriptors.SetSRV(0, Cmd.Image->RenderTexture ? *Cmd.Image->RenderTexture : *r_whitetexture->RenderTexture);
			}
			else
			{
				CallDescriptors.SetSRV(0, Cmd.SurfaceTexture ? *Cmd.SurfaceTexture : *r_whitetexture->RenderTexture);
			}
			CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex_DescriptorTable0, CallDescriptors.getRootDescriptorTableGpuHandle());

			g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

			FrameConstantBuffers::FrameConstantBuffer CB = ConstantBuffers.AllocateFrameConstantBuffer(sizeof(ColoredImageConstantBuffer));
			ColoredImageConstantBuffer* CBData = (ColoredImageConstantBuffer*)CB.getCPUMemory();
			CBData->OutputWidthAndInv[0] = float(BackBufferWidth);
			CBData->OutputWidthAndInv[1] = 1.0f / CBData->OutputWidthAndInv[0];
			CBData->OutputHeightAndInv[0] = float(BackBufferHeight);
			CBData->OutputHeightAndInv[1] = 1.0f / CBData->OutputHeightAndInv[0];
			CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());

			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			CommandList->DrawInstanced(Cmd.VertexCountPerInstance, Cmd.InstanceCount, Cmd.StartVertexLocation, Cmd.StartInstanceLocation);
		}

	}

private:

	RenderBufferGenericDynamic* ImageVertexRenderBuffer = nullptr;
	RenderBufferGenericDynamic* ImageIndexRenderBuffer = nullptr;

	bool bRecordingStarted;
	uint AllocatedVertexBytes;
	uint AllocatedIndexBytes;
	uint RecordedVertexCount;
	uint RecordedIndexCount;

	ImageVertexFormat* ImageVertexMemory;
	uint* ImageIndexMemory;

	ImageRenderCommand* RenderCommands;
	uint RecordedRenderCommandCount;
	ImageRenderCommand* CurrentCommand;
};

ImageRenderer* gImageRenderer = nullptr;


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////



void R_InitDrawImage(void)
{
	gImageRenderer = new ImageRenderer();

	ImageVertexFormatLayout.appendSimpleVertexDataToInputLayout("POSITION", 0, DXGI_FORMAT_R32G32_FLOAT);
	ImageVertexFormatLayout.appendSimpleVertexDataToInputLayout("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT);
	ImageVertexFormatLayout.appendSimpleVertexDataToInputLayout("TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
}

void DrawImageBeginFrame()
{
	gImageRenderer->StartRecording();
}

void AddDrawImage(DrawImageCall& DrawImageCall)
{
	gImageRenderer->ImageDraw(DrawImageCall);
}

void DrawAllImages(unsigned int BackBufferWidth, unsigned int BackBufferHeight)
{
	SCOPED_GPU_TIMER(DrawAllImages, 100, 100, 100);

	gImageRenderer->StopRecording();

	gImageRenderer->ExecuteRenderCommands(BackBufferWidth, BackBufferHeight);

/*
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
*/

	// Some rendering tests
#if 0
	{
		// Set PSO and render targets
		CachedRasterPsoDesc PSODesc;
		PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
		PSODesc.mLayout = nullptr;
		PSODesc.mVS = FullScreenTriangleVertexShader;
		PSODesc.mPS = UvPixelShader;
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

#if 0
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

		CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());

		CommandList->DrawInstanced(6, 1, 0, 0);
	}
#endif
}

void R_ShutdownDrawImage()
{
	delete gImageRenderer;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


