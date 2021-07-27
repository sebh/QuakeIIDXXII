// ref_dx12.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "dx_local.h"

// https://fabiensanglard.net/quake2/index.php

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

RenderTexture* DepthTexture = nullptr;

refimport_t	ri;
refdef_t	r_newrefdef;

model_t		*r_worldmodel;

const unsigned char* CinematicPalette = nullptr;

bool WorldMapLoaded = false;

cvar_t *vid_fullscreen;
cvar_t *vid_gamma;
cvar_t *vid_ref;

cvar_t *r_norefresh;
cvar_t *r_drawworld;
cvar_t *r_novis;
cvar_t *r_nocull;
cvar_t *r_speeds;
cvar_t *r_lightlevel;
cvar_t *r_drawentities;
cvar_t *r_lefthand;
cvar_t *r_lerpmodels;

cvar_t *gl_mode;	// Reusing that mode
cvar_t *gl_lockpvs;
cvar_t *gl_flashblend;
cvar_t *gl_modulate;
cvar_t *gl_monolightmap;
cvar_t *gl_polyblend;
cvar_t *gl_particle_size;
cvar_t *gl_dynamic;

int	r_framecount = 1;	// so frame counts initialized to 0 don't match
int	r_visframecount = 0;
int registration_sequence;

#if _DEBUGxxx
#define DEBUGPRINTF(A, B) {char text[128]; sprintf_s(text, 128, A, B); OutputDebugStringA(text);}
#define DEBUGPRINT(A) {OutputDebugStringA(A);}
#else
#define DEBUGPRINTF(A, B) 
#define DEBUGPRINT(A) 
#endif

void R_DX12_BeginRegistration(char* map)
{
	DEBUGPRINTF("R_DX12_BeginRegistration - %s\n", map);
	WorldMapLoaded = true;
	R_BeginRegistration(map);
}

struct model_s* R_DX12_RegisterModel(char* model)
{
	DEBUGPRINTF("R_DX12_RegisterModel - %s\n", model);

	model_t	*mod;
	int		i;
	dsprite_t	*sprout;
	dmdl_t		*pheader;

	mod = Mod_ForName(model, false);
	if (mod)
	{
		mod->registration_sequence = registration_sequence;

		// register any images used by the models
		if (mod->type == mod_sprite)
		{
			sprout = (dsprite_t *)mod->extradata;
			for (i = 0; i < sprout->numframes; i++)
				mod->skins[i] = FindImage(sprout->frames[i].name, it_sprite);
		}
		else if (mod->type == mod_alias)
		{
			pheader = (dmdl_t *)mod->extradata;
			for (i = 0; i < pheader->num_skins; i++)
				mod->skins[i] = FindImage((char *)pheader + pheader->ofs_skins + i * MAX_SKINNAME, it_skin);
			//PGM
			mod->numframes = pheader->num_frames;
			//PGM
		}
		else if (mod->type == mod_brush)
		{
	//		for (i = 0; i < mod->numtexinfo; i++)
	//			mod->texinfo[i].image->registration_sequence = registration_sequence;
		}
	}
	return mod;
}

struct image_s* R_DX12_RegisterSkin(char* skin)
{
	DEBUGPRINTF("R_DX12_RegisterSkin - %s\n", skin);
	return FindImage(skin, it_skin);
}

struct image_s* R_DX12_Draw_FindPic(char* name)
{
	DEBUGPRINTF("R_DX12_Draw_FindPic - %s\n", name);
	return Draw_FindPic(name);
}

void R_DX12_SetSky(char* name, float rotate, vec3_t axis)
{
	DEBUGPRINTF("R_DX12_SetSky - %s\n", name);
	SkyRegisterTexture(name, rotate, axis);
}

void R_DX12_EndRegistration(void)
{
	DEBUGPRINT("R_DX12_EndRegistration\n");
	R_EndRegistration();
}

void R_DX12_RenderFrame(refdef_t *fd)
{
	DEBUGPRINT("R_DX12_RenderFrame\n");
	// See R_RenderFrame

	r_newrefdef = *fd;

	if (r_norefresh->value || !r_drawworld->value)
	{
		return;
	}

	if (!WorldMapLoaded && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{
		ri.Sys_Error(ERR_DROP, "R_RenderView: NULL worldmodel");
	}

	R_RenderView();
}

void R_DX12_Draw_GetPicSize(int *w, int *h, char *name)
{
	image_t* Image = Draw_FindPic(name);
	if (Image)
	{
		*w = Image->width;
		*h = Image->height;
	}
	else
	{
		ri.Sys_Error(ERR_DROP, "R_DX12_Draw_GetPicSize - Could not find image %s", name);
	}
}

void R_DX12_Draw_Pic(int x, int y, char *name)
{
	DEBUGPRINTF("R_DX12_Draw_Pic - %s\n", name);
	DrawImageCall dic;
	dic.Image = Draw_FindPic(name);
	dic.Type = DrawImageCallType::Draw_Pic;
	dic.x = x;
	dic.y = y;
	dic.w = dic.Image->width;
	dic.h = dic.Image->height;
	AddDrawImage(dic);
}

void R_DX12_Draw_StretchPic(int x, int y, int w, int h, char *name)
{
	DEBUGPRINTF("R_DX12_Draw_StretchPic - %s\n", name);
	DrawImageCall dic;
	dic.Image = Draw_FindPic(name);
	dic.Type = DrawImageCallType::Draw_Pic;
	dic.x = x;
	dic.y = y;
	dic.w = w;
	dic.h = h;
	AddDrawImage(dic);
}

void R_DX12_Draw_Char(int x, int y, int c)
{
	DEBUGPRINTF("R_DX12_Draw_Char - %i\n", c);
	DrawImageCall dic;
	dic.Type = DrawImageCallType::Draw_Char;
	dic.x = x;
	dic.y = y;
	dic.c = c;
	AddDrawImage(dic);
}

void R_DX12_Draw_TileClear(int x, int y, int w, int h, char *name)
{
	DEBUGPRINTF("R_DX12_Draw_TileClear - %s\n", name);
	DrawImageCall dic;
	dic.Image = Draw_FindPic(name);
	dic.Type = DrawImageCallType::Draw_TileClear;
	dic.x = x;
	dic.y = y;
	dic.w = w;
	dic.h = h;
	AddDrawImage(dic);
}

void R_DX12_Draw_Fill(int x, int y, int w, int h, int c)
{
	DEBUGPRINTF("R_DX12_Draw_Fill - %i\n", c);
	DrawImageCall dic;
	dic.Type = DrawImageCallType::Draw_Fill;
	dic.x = x;
	dic.y = y;
	dic.w = w;
	dic.h = h;
	dic.c = c;
	AddDrawImage(dic);
}

void R_DX12_Draw_FadeScreen(void)
{
	DEBUGPRINT("R_DX12_Draw_FadeScreen\n");
	DrawImageCall dic;
	dic.Type = DrawImageCallType::Draw_FadeScreen;
	AddDrawImage(dic);
}

void R_DX12_Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte *data)
{
	DEBUGPRINT("R_DX12_Draw_StretchRaw\n");

	if (!CinematicPalette)
		return;

#if 0
	// This is used to render movies. Not supported yet so instead we print a grey background.
	DrawImageCall dic;
	dic.Type = DrawImageCallType::Draw_Fill;
	dic.x = x;
	dic.y = y;
	dic.w = w;
	dic.h = h;
	dic.c = 3;
	AddDrawImage(dic);
#else
	if (!MovieTextureDynamic || (MovieTextureDynamic && (MovieTextureDynamic->getWidth() != cols || MovieTextureDynamic->getHeight() != rows)))
	{
		if (MovieTextureDynamic)
		{
			delete MovieTextureDynamic;
		}
		MovieTextureDynamic = new RenderTextureDynamic(cols, rows, 1, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_NONE);
	}

	// Convert from color palette to rgba8 unorm
	byte* data32 = new byte[cols * rows * 4];
	for (int i = 0; i < cols * rows; ++i)
	{
		if (data[i] != 255)
		{
			data32[i * 4 + 0] = CinematicPalette[data[i]*3+0];
			data32[i * 4 + 1] = CinematicPalette[data[i]*3+1];
			data32[i * 4 + 2] = CinematicPalette[data[i]*3+2];
			data32[i * 4 + 3] = 255;
		}
		else
		{
			data32[i * 4 + 0] = 0;
			data32[i * 4 + 1] = 0;
			data32[i * 4 + 2] = 0;
			data32[i * 4 + 3] = 0;
		}
	}
	MovieTextureDynamic->Upload(data32, cols * 4, cols*rows * 4);
	delete [] data32;

	MovieTextureDynamic->getRenderTexture().resourceTransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	DrawImageCall dic;
	dic.Type = DrawImageCallType::Draw_Tex;
	dic.x = x;
	dic.y = y;
	dic.w = w;
	dic.h = h;
	dic.Texture = &MovieTextureDynamic->getRenderTexture();
	AddDrawImage(dic);
#endif
}

qboolean R_DX12_Init(void *hinstance, void *hWnd)
{
	// Get a bunch of cvar
	vid_fullscreen = ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = ri.Cvar_Get("vid_gamma", "1.0", CVAR_ARCHIVE);
	vid_ref = ri.Cvar_Get("vid_ref", "soft", CVAR_ARCHIVE);

	r_norefresh = ri.Cvar_Get("r_norefresh", "0", 0);
	r_drawworld = ri.Cvar_Get("r_drawworld", "1", 0);
	r_novis = ri.Cvar_Get("r_novis", "0", 0);
	r_nocull = ri.Cvar_Get("r_nocull", "0", 0);
	r_speeds = ri.Cvar_Get("r_speeds", "0", 0);
	r_lightlevel = ri.Cvar_Get("r_lightlevel", "0", 0);
	r_drawentities = ri.Cvar_Get("r_drawentities", "1", 0);
	r_lefthand = ri.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	r_lerpmodels = ri.Cvar_Get("r_lerpmodels", "1", 0);

	gl_mode = ri.Cvar_Get("gl_mode", "3", CVAR_ARCHIVE);
	gl_lockpvs = ri.Cvar_Get("gl_lockpvs", "0", 0);
	gl_flashblend = ri.Cvar_Get("gl_flashblend", "0", 0);
	gl_modulate = ri.Cvar_Get("gl_modulate", "1", CVAR_ARCHIVE);
	gl_monolightmap = ri.Cvar_Get("gl_monolightmap", "0", 0);
	gl_polyblend = ri.Cvar_Get("gl_polyblend", "1", 0);
	gl_particle_size = ri.Cvar_Get("gl_particle_size", "40", CVAR_ARCHIVE);
	gl_dynamic = ri.Cvar_Get("gl_dynamic", "1", 0);

	registration_sequence = 1;

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
	RECT SavedClientRect = vid.ClientRect;
	AdjustWindowRect(&vid.ClientRect, style, menu);
	//Get the required window dimensions
	vid.width = vid.ClientRect.right - vid.ClientRect.left; //Required width
	vid.height = vid.ClientRect.bottom - vid.ClientRect.top; //Required height

	// Re-adjust position to not have the window going up after a few run
	LONG DeltaX = SavedClientRect.left - vid.ClientRect.left;
	LONG DeltaY = SavedClientRect.top - vid.ClientRect.top;
	vid.ClientRect.right  += DeltaX;
	vid.ClientRect.bottom += DeltaY;
	vid.ClientRect.left   += DeltaX;
	vid.ClientRect.top    += DeltaY;

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
	Dx12Device::initialise(vid.HWnd, vid.width, vid.height);
	CachedPSOManager::initialise();

	if (g_dx12Device->getDevice() == nullptr)
	{
		ri.Con_Printf(PRINT_ALL, "DX12 device creation failed\n");
		return false;
	}
	ID3D12Resource* backBuffer = g_dx12Device->getBackBuffer();

	// Create the depth buffer
	D3D12_CLEAR_VALUE DepthClearValue;
	DepthClearValue.Format = DXGI_FORMAT_R24G8_TYPELESS;
	DepthClearValue.DepthStencil.Depth = 1.0f;
	DepthClearValue.DepthStencil.Stencil = 0;
	DepthTexture = new RenderTexture(
		(uint)backBuffer->GetDesc().Width, (uint)backBuffer->GetDesc().Height, 1,
		DepthClearValue.Format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &DepthClearValue);

	SetForegroundWindow(vid.HWnd);
	SetFocus(vid.HWnd);

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow(vid.width, vid.height);

	LoadAllShaders();

	R_InitRenderView();

	return true;
}

void R_DX12_Shutdown(void)
{
	g_dx12Device->closeBufferedFramesBeforeShutdown();

	UnloadAllShaders();

	ReleaseAllStates();

	Mod_FreeAll();

	SkyUnregisterTexture();

	R_ShutdownRenderView();
	UnloadAllTextures();

	delete DepthTexture;

	CachedPSOManager::shutdown();
	Dx12Device::shutdown();

	if (vid.HWnd)
	{
		DestroyWindow(vid.HWnd);
		vid.HWnd = NULL;
	}
	UnregisterClass(vid.Wc.lpszClassName, vid.hInstance);
	vid.hInstance = NULL;
	vid.wndproc = NULL;
}

void R_DX12_BeginFrame(float camera_separation)
{
	DEBUGPRINT("R_DX12_BeginFrame\n");
	//	SCOPED_GPU_TIMER(BeginFrame, 100, 100, 100);
	g_dx12Device->beginFrame();

	{
		SCOPED_GPU_TIMER(BeginFrame, 100, 100, 100);

		if (!AreAllStatesCreated())
		{
			CreateAllStates();
		}

		UploadAllTextures();

		DrawBeginFrame();

		// Get device resources
		ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
		ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();
		D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptor = g_dx12Device->getBackBufferDescriptor();
		float AspectRatioXOverY = float(BackBuffer->GetDesc().Width) / float(BackBuffer->GetDesc().Height);

		// Make back buffer targetable and set it
		D3D12_RESOURCE_BARRIER BarrierPresentToRt = {};
		BarrierPresentToRt.Transition.pResource = BackBuffer;
		BarrierPresentToRt.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierPresentToRt.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierPresentToRt.Transition.Subresource = 0;
		CommandList->ResourceBarrier(1, &BarrierPresentToRt);

		// We do not always clear because the game only update parts of the screen that are needed for isntance hwen reducing the viewport size.
//		FLOAT BackBufferClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//		CommandList->ClearRenderTargetView(BackBufferDescriptor, BackBufferClearColor, 0, nullptr);
	}
}

void R_DX12_EndFrame(void)
{
	{
		SCOPED_GPU_TIMER(EndFrame, 100, 100, 100);

		// Get device resources
		ID3D12GraphicsCommandList* CommandList = g_dx12Device->getFrameCommandList();
		ID3D12Resource* BackBuffer = g_dx12Device->getBackBuffer();

		UploadAllTextures();

		// Draw_pic and co are called after RenderFrame so image draw must happen here
		DrawAllImages(vid.width, vid.height);

		// Make back-buffer presentable.
		D3D12_RESOURCE_BARRIER BarrierRtToPresent = {};
		BarrierRtToPresent.Transition.pResource = BackBuffer;
		BarrierRtToPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierRtToPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		BarrierRtToPresent.Transition.Subresource = 0;
		CommandList->ResourceBarrier(1, &BarrierRtToPresent);
	}

	DEBUGPRINT("R_DX12_EndFrame\n");
	//	SCOPED_GPU_TIMER(EndFrame, 100, 100, 100);
	const bool VSyncEnabled = true;
	g_dx12Device->endFrameAndSwap(VSyncEnabled);
}

void R_DX12_CinematicSetPalette(const unsigned char *palette)
{
	DEBUGPRINT("R_DX12_CinematicSetPalette\n");
	CinematicPalette = palette;	// We assume the pointer can safely be kept around and always specified before the movie is played.
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

