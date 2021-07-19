
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
	int		registration_sequence;		// 0 = free
	struct msurface_s	*texturechain;	// for sort-by-texture world drawing
	int		texnum;						// texture binding
	bool	has_alpha;

	RenderTexture* RenderTexture;
} image_t;

image_t	*Draw_FindPic(char *name);


void UploadAllTextures();
void UnloadAllTextures();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders

void LoadAllShaders();
void UnloadAllShaders();


extern VertexShader* ImageDrawVertexShader;
extern PixelShader*  ImageDrawPixelShader;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// States

bool AreAllStatesCreated();
void CreateAllStates();
void ReleaseAllStates();

extern RenderBufferGeneric* IndexBuffer;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

enum class DrawImageCallType
{
	Draw_Pic,
	Draw_StretchPic,
	Draw_Char,
	Draw_TileClear,
	Draw_Fill,
};

struct DrawImageCall
{
	DrawImageCallType Type;
	image_t* Image;

	struct Draw_Pic
	{
		int x;
		int y;
	};
	struct Draw_StretchPic
	{
		int x;
		int y;
		int w;
		int h;
	};
	struct Draw_Char
	{
		int x;
		int y;
		int c;
	};
	struct Draw_TileClear
	{
		int x;
		int y;
		int w;
		int h;
	};
	struct Draw_Fill
	{
		int x;
		int y;
		int w;
		int h;
		int c;
	};

	union
	{
		Draw_Pic		DrawPic;
		Draw_StretchPic	DrawStretchPic;
		Draw_Char		DrawChar;
		Draw_TileClear	DrawTileClear;
		Draw_Fill		DrawFill;
	};
};

void DrawBeginFrame();
void AddDrawImage(DrawImageCall& DrawImageCall);
void DrawAllImages(unsigned int BackBufferWidth, unsigned int BackBufferHeight);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Others

extern	refimport_t	ri;


