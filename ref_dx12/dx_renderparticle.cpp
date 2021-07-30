
#include "dx_local.h"
#include "stdlib.h"



static InputLayout ParticleVertexFormatLayout;

// Current simplification on purpose: all world/mesh draw are using the same vertex layout
struct ParticleFormat
{
	float Position[3];
	float ColorAlpha[4];
};

RenderBufferGenericDynamic* ParticleRenderBuffer = nullptr;


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////




void R_InitParticleRenderer(void)
{
	ParticleRenderBuffer = new RenderBufferGenericDynamic(sizeof(ParticleFormat) * MAX_PARTICLES, D3D12_RESOURCE_FLAG_NONE);

	ParticleVertexFormatLayout.appendPerInstanceVertexDataToInputLayout("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1);
	ParticleVertexFormatLayout.appendPerInstanceVertexDataToInputLayout("TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1);

}

void R_RenderParticles(void)
{
	SCOPED_GPU_TIMER(RenderPArticles, 100, 100, 100);

	ViewData vd = GetViewData();

	// Convert particle data to our particle buffer
	ParticleFormat* Particles = (ParticleFormat*) ParticleRenderBuffer->Map();
	unsigned char color[4];
	for (int i = 0; i < r_newrefdef.num_particles; ++i)
	{
		*(int *)color = d_8to24table[r_newrefdef.particles[i].color];

		Particles[i].Position[0] = r_newrefdef.particles[i].origin[0];
		Particles[i].Position[1] = r_newrefdef.particles[i].origin[1];
		Particles[i].Position[2] = r_newrefdef.particles[i].origin[2];
		Particles[i].ColorAlpha[0] = float(color[0]) * 1.0f/255.0f;
		Particles[i].ColorAlpha[1] = float(color[1]) * 1.0f/255.0f;
		Particles[i].ColorAlpha[2] = float(color[2]) * 1.0f/255.0f;
		Particles[i].ColorAlpha[3] = r_newrefdef.particles[i].alpha;
	}
	ParticleRenderBuffer->UnmapAndUpload();

	// Render particles as instanced quads
	ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
	ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();

	FrameConstantBuffers& ConstantBuffers = g_dx12Device->getFrameConstantBuffers();
	DispatchDrawCallCpuDescriptorHeap& DrawDispatchCallCpuDescriptorHeap = g_dx12Device->getDispatchDrawCallCpuDescriptorHeap();

	// Set defaults graphic and compute root signatures
	CommandList->SetGraphicsRootSignature(g_dx12Device->GetDefaultGraphicRootSignature().getRootsignature());
	CommandList->SetComputeRootSignature(g_dx12Device->GetDefaultComputeRootSignature().getRootsignature());

	// Set the common descriptor heap
	std::vector<ID3D12DescriptorHeap*> descriptorHeaps;
	descriptorHeaps.push_back(g_dx12Device->getFrameDispatchDrawCallGpuDescriptorHeap()->getHeap());
	CommandList->SetDescriptorHeaps(uint(descriptorHeaps.size()), descriptorHeaps.data());

	CachedRasterPsoDesc PSODesc;
	PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
	PSODesc.mLayout = &ParticleVertexFormatLayout;
	PSODesc.mBlendState = &getBlendState_AlphaBlending();
	PSODesc.mDepthStencilState = &getDepthStencilState_ReadOnly();
	PSODesc.mRasterizerState = &getRasterizerState_DefaultNoCulling();
	PSODesc.mRenderTargetCount = 1;
	PSODesc.mRenderTargetDescriptors[0] = BackBufferDescriptor;
	PSODesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PSODesc.mDepthTextureDescriptor = DepthTexture->getDSVCPUHandle();
	PSODesc.mDepthTextureFormat = DepthTexture->getClearColor().Format;
	PSODesc.mVS = ParticleVertexShader;
	PSODesc.mPS = ParticlePixelShader;

	ParticleRenderBuffer->getRenderBuffer().resourceTransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	D3D12_VERTEX_BUFFER_VIEW MeshVertexRenderBufferView = ParticleRenderBuffer->getRenderBuffer().getVertexBufferView(sizeof(ParticleFormat));
	CommandList->IASetVertexBuffers(0, 1, &MeshVertexRenderBufferView);

	g_CachedPSOManager->SetPipelineState(CommandList, PSODesc);

	FrameConstantBuffers::FrameConstantBuffer CB = ConstantBuffers.AllocateFrameConstantBuffer(sizeof(ParticleConstantBuffer));
	ParticleConstantBuffer* CBData = (ParticleConstantBuffer*)CB.getCPUMemory();
	CBData->ViewProjectionMatrix = vd.ViewProjectionMatrix;
	CBData->ViewUp[0] = vd.up[0];
	CBData->ViewUp[1] = vd.up[1];
	CBData->ViewUp[2] = vd.up[2];
	CBData->ViewRight[0] = vd.right[0];
	CBData->ViewRight[1] = vd.right[1];
	CBData->ViewRight[2] = vd.right[2];
	CBData->ParticleSize = gl_particle_size->value * 0.025f; // Could implement https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPointSize.xml but a world space size will do for now.
	CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex_CBV0, CB.getGPUVirtualAddress());

	CommandList->DrawInstanced(6, r_newrefdef.num_particles, 0, 0);
}

void R_ShutdownParticleRenderer(void)
{
	if (ParticleRenderBuffer)
	{
		delete ParticleRenderBuffer;
		ParticleRenderBuffer = nullptr;
	}
}

