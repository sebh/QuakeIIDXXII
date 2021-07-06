// ref_dx12.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


extern "C"
{
	#include "../client/ref.h"
}


void R_DX12_BeginRegistration(char* map)
{
	// TODO
	int i = 0;
}

struct model_s* R_DX12_RegisterModel(char* model)
{
	// TODO
	int i = 0;
	return nullptr;
}

struct image_s* R_DX12_RegisterSkin(char* skin)
{
	return nullptr;
}

struct image_s* R_DX12_Draw_FindPic(char* name)
{
	// TODO
	int i = 0;
	return nullptr;
}

void R_DX12_SetSky(char* name, float rotate, vec3_t axis)
{
	// TODO
	int i = 0;
}

void R_DX12_EndRegistration(void)
{
	// TODO
	int i = 0;
}

void R_DX12_RenderFrame(refdef_t *fd)
{
	// TODO
	int i = 0;
}

void R_DX12_Draw_GetPicSize(int *w, int *h, char *name)
{
	// TODO
	int i = 0;
}

void R_DX12_Draw_Pic(int x, int y, char *name)
{
	// TODO
	int i = 0;
}

void R_DX12_Draw_StretchPic(int x, int y, int w, int h, char *name)
{
	// TODO
	int i = 0;
}

void R_DX12_Draw_Char(int x, int y, int c)
{
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
	// TODO
	return true;
}

void R_DX12_Shutdown(void)
{
	// TODO
	int i = 0;
}

void R_DX12_BeginFrame(float camera_separation)
{
	// TODO
	int i = 0;
}

void R_DX12_EndFrame(void)
{
	// TODO
	int i = 0;
}

void R_DX12_CinematicSetPalette(const unsigned char *palette)
{
	// TODO
	int i = 0;
}

void R_DX12_AppActivate(qboolean active)
{
	// TODO
	int i = 0;
}



refimport_t	ri;

refexport_t GetRefAPI(refimport_t rimp)
{
	refexport_t	re;

	ri = rimp;

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

