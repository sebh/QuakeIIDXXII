
extern "C"
{
#include "../client/ref.h"
}

#include "Dx12Device.h"


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


extern VertexShader* TestVertexShader;
extern PixelShader*  TestPixelShader;

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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// States

bool AreAllStatesCreated();
void CreateAllStates();
void ReleaseAllStates();

extern RenderBufferGeneric* IndexBufferSingleTri;
extern RenderBufferGeneric* IndexBufferQuadTris;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

enum class DrawImageCallType
{
	Draw_Pic,		// Draw an image
	Draw_Char,		// Draw a character
	Draw_TileClear,	// tile the image over 64x64 pixel tiles
	Draw_Fill,		// fill with a single color
};

struct DrawImageCall
{
	DrawImageCallType Type;

	image_t* Image;
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

void SkyRegisterTexture(char* name, float rotate, vec3_t axis);
void SkyUnregisterTexture();
void SkyRender();



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Others

extern refimport_t ri;

extern refdef_t r_newrefdef;

void ErrorExit(char* Text);


