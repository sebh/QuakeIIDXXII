// ref_dx12.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "dx_local.h"

#include "Dx12Device.h"

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

struct WinDx12State
{
	HINSTANCE	hInstance;
	WNDPROC		wndproc;

	HWND		HWnd;			/// The handle for the window, filled by a function
	WNDCLASSEX	Wc;				/// This struct holds information for the window class
	RECT		ClientRect;		/// The client rectangle where we render into
	unsigned int width;
	unsigned int height;
};

WinDx12State vid;

refimport_t	ri;

refdef_t	refdef;

bool WorldMapLoaded = false;

cvar_t *gl_mode;	// Reusing that mode
cvar_t *vid_fullscreen;
cvar_t *vid_gamma;
cvar_t *vid_ref;
cvar_t *r_norefresh;

#if _DEBUG
#define DEBUGPRINTF(A, B) {char text[128]; sprintf_s(text, 128, A, B); OutputDebugStringA(text);}
#define DEBUGPRINT(A) {OutputDebugStringA(A);}
#else
#define DEBUGPRINTF(A, B) 
#define DEBUGPRINT(A) 
#endif

void R_DX12_BeginRegistration(char* map)
{
	DEBUGPRINTF("R_DX12_BeginRegistration - %s\n", map);
	// TODO
	int i = 0;

	WorldMapLoaded = true;
}

struct model_s* R_DX12_RegisterModel(char* model)
{
	DEBUGPRINTF("R_DX12_RegisterModel - %s\n", model);
	// TODO
	int i = 0;
	return nullptr;
}

struct image_s* R_DX12_RegisterSkin(char* skin)
{
	DEBUGPRINTF("R_DX12_RegisterSkin - %s\n", skin);
	return nullptr;
}

struct image_s* R_DX12_Draw_FindPic(char* name)
{
	DEBUGPRINTF("R_DX12_Draw_FindPic - %s\n", name);
	// TODO
	int i = 0;
	return nullptr;
}

void R_DX12_SetSky(char* name, float rotate, vec3_t axis)
{
	DEBUGPRINTF("R_DX12_SetSky - %s\n", name);
	// TODO
	int i = 0;
}

void R_DX12_EndRegistration(void)
{
	DEBUGPRINT("R_DX12_EndRegistration\n");
	// TODO
	int i = 0;
}

void R_DX12_RenderFrame(refdef_t *fd)
{
	DEBUGPRINT("R_DX12_RenderFrame\n");
	// See R_RenderFrame

	if (r_norefresh->value)
		return;

	refdef = *fd;

	if (!WorldMapLoaded && !(refdef.rdflags & RDF_NOWORLDMODEL))
	{
		ri.Sys_Error(ERR_DROP, "R_RenderView: NULL worldmodel");
	}

	SCOPED_GPU_TIMER(Quake2Frame, 100, 100, 100);

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
	Viewport.Width = vid.width;
	Viewport.Height = vid.height;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	CommandList->RSSetViewports(1, &Viewport);
	D3D12_RECT ScissorRect;
	ScissorRect.left = 0;
	ScissorRect.top = 0;
	ScissorRect.right = vid.width;
	ScissorRect.bottom = vid.height;
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Make back buffer targetable and set it
	D3D12_RESOURCE_BARRIER BarrierPresentToRt = {};
	BarrierPresentToRt.Transition.pResource = BackBuffer;
	BarrierPresentToRt.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierPresentToRt.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierPresentToRt.Transition.Subresource = 0;
	CommandList->ResourceBarrier(1, &BarrierPresentToRt);

	FLOAT BackBufferClearColor[4] = { 1.0f, 0.5f, 0.5f, 1.0f };
	CommandList->ClearRenderTargetView(BackBufferDescriptor, BackBufferClearColor, 0, nullptr);

	// Some rendering tests
	{
		// Set PSO and render targets
		CachedRasterPsoDesc PSODesc;
		PSODesc.mRootSign = &g_dx12Device->GetDefaultGraphicRootSignature();
		PSODesc.mLayout = NullInputLayout;
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

		IndexBuffer->resourceTransitionBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER);
		D3D12_INDEX_BUFFER_VIEW IndexBufferView = IndexBuffer->getIndexBufferView(DXGI_FORMAT_R32_UINT);
		CommandList->IASetIndexBuffer(&IndexBufferView);

		CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
	}

	// Make back-buffer presentable.
	D3D12_RESOURCE_BARRIER BarrierRtToPresent = {};
	BarrierRtToPresent.Transition.pResource = BackBuffer;
	BarrierRtToPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierRtToPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	BarrierRtToPresent.Transition.Subresource = 0;
	CommandList->ResourceBarrier(1, &BarrierRtToPresent);
}

void R_DX12_Draw_GetPicSize(int *w, int *h, char *name)
{
	// TODO
	int i = 0;
}

void R_DX12_Draw_Pic(int x, int y, char *name)
{
	DEBUGPRINTF("R_DX12_Draw_Pic - %s\n", name);
	// TODO
	int i = 0;
}

void R_DX12_Draw_StretchPic(int x, int y, int w, int h, char *name)
{
	DEBUGPRINTF("R_DX12_Draw_StretchPic - %s\n", name);
	// TODO
	int i = 0;
}

void R_DX12_Draw_Char(int x, int y, int c)
{
	DEBUGPRINTF("R_DX12_Draw_Char - %i\n", c);
	// TODO
	int i = 0;
}

void R_DX12_Draw_TileClear(int x, int y, int w, int h, char *name)
{
	// TODO
	int i = 0;
}

void R_DX12_Draw_Fill(int x, int y, int w, int h, int c)
{
	// TODO
	int i = 0;
}

void R_DX12_Draw_FadeScreen(void)
{
	// TODO
	int i = 0;
}

void R_DX12_Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte *data)
{
	// TODO
	int i = 0;
}

qboolean R_DX12_Init(void *hinstance, void *hWnd)
{
	// Get a bunch of cvar
	vid_fullscreen = ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = ri.Cvar_Get("vid_gamma", "1.0", CVAR_ARCHIVE);
	vid_ref = ri.Cvar_Get("vid_ref", "soft", CVAR_ARCHIVE);
	gl_mode = ri.Cvar_Get("gl_mode", "3", CVAR_ARCHIVE);
	r_norefresh = ri.Cvar_Get("r_norefresh", "0", 0);

	qboolean fullscreen = vid_fullscreen->value;

	int width, height;
	const char *win_fs[] = { "Windowed", "Fullscreen" };

	ri.Con_Printf(PRINT_ALL, "Initializing DX12 display\n");

	if (!ri.Vid_GetModeInfo(&width, &height, gl_mode->value))
	{
		ri.Con_Printf(PRINT_ALL, " invalid mode\n");
		return rserr_invalid_mode;
	}

	ri.Con_Printf(PRINT_ALL, " %d %d %s\n", width, height, win_fs[fullscreen ? 1 : 0]);

	vid.height = height;
	vid.width = width;
	vid.wndproc = (WNDPROC)hWnd;
	vid.hInstance = (HINSTANCE)hinstance;

	// destroy the existing window
	if (vid.HWnd)
	{
		DestroyWindow(vid.HWnd);
		vid.HWnd = NULL;
	}

	vid.ClientRect.left = -1;
	vid.ClientRect.top = -1;
	if (fullscreen)
	{
		vid.ClientRect.left = 0;
		vid.ClientRect.top = 0;
	}
	else
	{
		vid.ClientRect.left = ri.Cvar_Get("vid_xpos", "0", 0)->value;
		vid.ClientRect.top = ri.Cvar_Get("vid_ypos", "0", 0)->value;
	}
	vid.ClientRect.right = vid.ClientRect.left + vid.width;
	vid.ClientRect.bottom = vid.ClientRect.top + vid.height;


	DWORD style = 0;// (WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX); // WS_OVERLAPPEDWINDOW, add simple style as paramter
	int exstyle;
	if (fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		style = WS_POPUP | WS_VISIBLE;
	}
	else
	{
		exstyle = 0;
		style = (WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE);
	}

	// And create the rectangle that will allow it
	BOOL menu = false;
	AdjustWindowRect(&vid.ClientRect, style, menu);
	//Get the required window dimensions
	vid.width = vid.ClientRect.right - vid.ClientRect.left; //Required width
	vid.height = vid.ClientRect.bottom - vid.ClientRect.top; //Required height

	// clear out the window class for use
	ZeroMemory(&vid.Wc, sizeof(WNDCLASSEX));
	// fill in the struct with the needed information
	vid.Wc.cbSize = sizeof(WNDCLASSEX);
	vid.Wc.cbClsExtra = 0;
	vid.Wc.cbWndExtra = 0;
	vid.Wc.hIcon = 0;
	vid.Wc.lpszMenuName = 0;
	vid.Wc.style = CS_HREDRAW | CS_VREDRAW;
	vid.Wc.lpfnWndProc = vid.wndproc;
	vid.Wc.hInstance = vid.hInstance;
	vid.Wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	vid.Wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
	vid.Wc.lpszClassName = L"Quake 2 Class Name";

	// register the window class
	RegisterClassEx(&vid.Wc);

	// create the window and use the result as the handle
	vid.HWnd = CreateWindowEx(
		NULL,
		vid.Wc.lpszClassName,				// name of the window class
		L"Quake 2",							// title of the window
		style,								// not resizable
		vid.ClientRect.left,				// x-position of the window
		vid.ClientRect.top,					// y-position of the window
		vid.width,							// width of the window
		vid.height,							// height of the window
		NULL,								// we have no parent window, NULL
		NULL,								// we aren't using menus, NULL
		vid.hInstance,						// application handle
		NULL);								// used with multiple windows, NULL

	ShowWindow(vid.HWnd, SW_SHOW);
	UpdateWindow(vid.HWnd);

	// Create the d3d device 
	Dx12Device::initialise(vid.HWnd);
	CachedPSOManager::initialise();

	if (g_dx12Device->getDevice() == nullptr)
	{
		ri.Con_Printf(PRINT_ALL, "DX12 device creation failed\n");
		return false;
	}

	SetForegroundWindow(vid.HWnd);
	SetFocus(vid.HWnd);

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow(vid.width, vid.height);

	LoadAllShaders();

	return true;
}

void R_DX12_Shutdown(void)
{
	g_dx12Device->closeBufferedFramesBeforeShutdown();

	UnloadAllShaders();
	ReleaseAllStates();

	CachedPSOManager::shutdown();
	Dx12Device::shutdown();
}

void R_DX12_BeginFrame(float camera_separation)
{
	DEBUGPRINT("R_DX12_BeginFrame\n");
	//	SCOPED_GPU_TIMER(BeginFrame, 100, 100, 100);
	g_dx12Device->beginFrame();

	if (!AreAllStatesCreated())
	{
		CreateAllStates();
	}
}

void R_DX12_EndFrame(void)
{
	DEBUGPRINT("R_DX12_EndFrame\n");
	//	SCOPED_GPU_TIMER(EndFrame, 100, 100, 100);
	const bool VSyncEnabled = true;
	g_dx12Device->endFrameAndSwap(VSyncEnabled);
}

void R_DX12_CinematicSetPalette(const unsigned char *palette)
{
	// TODO
	int i = 0;
}

void R_DX12_AppActivate(qboolean active)
{
	if (active)
	{
		SetForegroundWindow(vid.HWnd);
		ShowWindow(vid.HWnd, SW_RESTORE);
	}
	else
	{
		if (vid_fullscreen->value)
			ShowWindow(vid.HWnd, SW_MINIMIZE);
	}
}

refexport_t GetRefAPI(refimport_t rimp)
{
	refexport_t	re;

	ri = rimp;
	memset(&vid, 0, sizeof(WinDx12State));

	re.api_version = API_VERSION;

	re.BeginRegistration = R_DX12_BeginRegistration;
	re.RegisterModel = R_DX12_RegisterModel;
	re.RegisterSkin = R_DX12_RegisterSkin;
	re.RegisterPic = R_DX12_Draw_FindPic;
	re.SetSky = R_DX12_SetSky;
	re.EndRegistration = R_DX12_EndRegistration;

	re.RenderFrame = R_DX12_RenderFrame;

	re.DrawGetPicSize = R_DX12_Draw_GetPicSize;
	re.DrawPic = R_DX12_Draw_Pic;
	re.DrawStretchPic = R_DX12_Draw_StretchPic;
	re.DrawChar = R_DX12_Draw_Char;
	re.DrawTileClear = R_DX12_Draw_TileClear;
	re.DrawFill = R_DX12_Draw_Fill;
	re.DrawFadeScreen = R_DX12_Draw_FadeScreen;

	re.DrawStretchRaw = R_DX12_Draw_StretchRaw;

	re.Init = R_DX12_Init;
	re.Shutdown = R_DX12_Shutdown;

	re.CinematicSetPalette = R_DX12_CinematicSetPalette;
	re.BeginFrame = R_DX12_BeginFrame;
	re.EndFrame = R_DX12_EndFrame;

	re.AppActivate = R_DX12_AppActivate;

	Swap_Init();

	return re;
}



#ifndef REF_HARD_LINKED
extern "C"
{

	// this is only here so the functions in q_shared.c and q_shwin.c can link

	void Sys_Error(char *error, ...)
	{
		va_list		argptr;
		char		text[1024];

		va_start(argptr, error);
		vsprintf_s(text, 1024, error, argptr);
		va_end(argptr);

		const char* Format = "%s";
		ri.Sys_Error(ERR_FATAL, const_cast<char*>(Format), text);
	}

	void Com_Printf(char *fmt, ...)
	{
		va_list		argptr;
		char		text[1024];

		va_start(argptr, fmt);
		vsprintf_s(text, 1024, fmt, argptr);
		va_end(argptr);

		const char* Format = "%s";
		ri.Con_Printf(PRINT_ALL, const_cast<char*>(Format), text);
	}

}
#endif

