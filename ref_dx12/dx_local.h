
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
} image_t;

image_t	*Draw_FindPic(char *name);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders

void LoadAllShaders();
void UnloadAllShaders();


extern VertexShader* ImageDrawVertexShader;
extern PixelShader*  ImageDrawPixelShader;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// States

void CreateAllStates();
void ReleaseAllStates();

extern InputLayout* NullInputLayout;

extern RenderBufferGeneric* vertexBuffer;
extern RenderBufferGeneric* indexBuffer;

