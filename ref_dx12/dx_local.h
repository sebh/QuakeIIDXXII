
extern "C"
{
#include "../client/ref.h"
}

#include "Dx12Device.h"
#include "Dx12Utilities.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Images

/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

*/
typedef enum
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky
} imagetype_t;

typedef struct image_s
{
	char	name[MAX_QPATH];			// game path, including extension
	imagetype_t	type;
	int		width, height;				// source image
	int		upload_width, upload_height;	// after power of two and picmip
//	int		registration_sequence;		// 0 = free
//	struct msurface_s	*texturechain;	// for sort-by-texture world drawing

	bool	uploaded;
	bool	has_alpha;
	int		bits;
	byte*	pic;
	RenderTexture* RenderTexture;
} image_t;

image_t* FindImage(char *name, imagetype_t type);
image_t	*Draw_FindPic(char *name);

extern image_t* r_notexture;
extern image_t* r_charstexture;
extern unsigned d_8to24table[256];

void UploadAllTextures();
void UnloadAllTextures();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders

void LoadAllShaders();
void UnloadAllShaders();


extern VertexShader* FullScreenTriangleVertexShader;
extern PixelShader*  UvPixelShader;

struct ImageDrawConstantBuffer
{
	float	OutputWidthAndInv[2];
	float	OutputHeightAndInv[2];
	float	ImageBottomLeft[2];
	float	ImageSize[2];
	float	ColorAlpha[4];
};
extern VertexShader* ImageDrawVertexShader;
extern PixelShader*  ImageDrawPixelShader;
extern PixelShader*  TiledImageDrawPixelShader;
extern PixelShader*  ColorDrawPixelShader;
extern PixelShader*  CharDrawPixelShader;

struct SkyConstantBuffer
{
	float4x4	ViewProjectionMatrix;
	uint		Face;
	uint		pad0[3];
};
extern VertexShader* SkyVertexShader;
extern PixelShader*  SkyPixelShader;

struct MeshConstantBuffer
{
	float4x4	MeshWorldMatrix;
	float4x4	ViewProjectionMatrix;
};
extern VertexShader* MeshVertexShader;
extern PixelShader* MeshDebugPixelShader;
extern PixelShader* MeshColorPixelShader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// States

bool AreAllStatesCreated();
void CreateAllStates();
void ReleaseAllStates();

extern RenderBufferGeneric* IndexBufferSingleTri;
extern RenderBufferGeneric* IndexBufferQuadTris;

extern RenderTextureDynamic* MovieTextureDynamic;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

enum class DrawImageCallType
{
	Draw_Pic,		// Draw an image
	Draw_Tex,		// Draw a RenderTexture
	Draw_Char,		// Draw a character
	Draw_TileClear,	// tile the image over 64x64 pixel tiles
	Draw_Fill,		// fill with a single color
	Draw_FadeScreen
};

struct DrawImageCall
{
	DrawImageCallType Type;

	union {
		image_t* Image;
		RenderTexture* Texture;
	};
	int c;

	int x;
	int y;
	int w;
	int h;

	DrawImageCall()
	{
		Type = DrawImageCallType::Draw_Pic;
		Image = nullptr;
		c = -1;
		x = -1;
		y = -1;
		w = -1;
		h = -1;
	}
};

void DrawBeginFrame();
void AddDrawImage(DrawImageCall& DrawImageCall);
void DrawAllImages(unsigned int BackBufferWidth, unsigned int BackBufferHeight);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render view

void R_InitRenderView(void);
void R_RenderView(void);
void R_ShutdownRenderView(void);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light

void R_LightPoint(vec3_t p, vec3_t color);
void R_PushDlights(void);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Surfaces

void R_MarkLeaves(void);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Model and draw

#define MAX_LBM_HEIGHT  480

#define BACKFACE_EPSILON	0.01

#include "dx_gl_model.h"

extern int r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;
extern entity_t *currententity;
extern model_t *currentmodel;
extern model_t	*r_worldmodel;


void R_BeginRegistration(char *model);
void R_EndRegistration(void);
void DrawEntities();



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sky

void SkyRegisterTexture(char* name, float rotate, vec3_t axis);
void SkyUnregisterTexture();
void SkyRender();



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Others

extern refimport_t ri;

extern refdef_t r_newrefdef;

void ErrorExit(char* Text);

extern cvar_t *r_novis;
extern cvar_t *r_speeds;
extern cvar_t *r_lightlevel;
extern cvar_t *r_drawentities;

extern cvar_t *gl_lockpvs;
extern cvar_t *gl_flashblend;
extern cvar_t *gl_modulate;
extern cvar_t *gl_monolightmap;
extern cvar_t *gl_polyblend;

extern int r_visframecount;
extern int r_framecount;
extern int r_dlightframecount;

extern int registration_sequence;
extern int c_visible_lightmaps;
extern int c_visible_textures;

struct ViewData
{
	float4x4 ViewMatrix;
	float4x4 ViewMatrixInv;
	float4x4 ProjectionMatrix;
	float4x4 ProjectionMatrixInv;
	float4x4 ViewProjectionMatrix;
	float4x4 ViewProjectionMatrixInv;
};
// ignoring VR views
ViewData GetViewData();


