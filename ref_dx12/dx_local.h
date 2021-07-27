
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

extern image_t* r_whitetexture;
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
extern PixelShader* MeshLightmapSurfacePixelShader;
extern PixelShader* MeshColoredSurfacePixelShader;

struct ParticleConstantBuffer
{
	float4x4	ViewProjectionMatrix;
	float		ViewRight[4];
	float		ViewUp[4];
	float		ParticleSize;
	float		Pad0[3];
};
extern VertexShader* ParticleVertexShader;
extern PixelShader* ParticlePixelShader;

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

void R_MarkLights(dlight_t *light, int bit, /*mnode_t*/void *node);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Surfaces

void R_MarkLeaves(void);
void R_DrawWorld();
void R_DrawBrushModel(entity_t *e);

#define	MAX_LIGHTMAPS	128
extern RenderTextureDynamic* Dx12Lightmaps[MAX_LIGHTMAPS];



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Particles

void R_InitParticleRenderer(void);
void R_RenderParticles(void);
void R_ShutdownParticleRenderer(void);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Model and draw

void R_DrawAliasModel(entity_t *e);

extern float4x4 LastEntityWorldMatrix;
void R_RotateForEntity(entity_t *e);
void R_SetIdentityTransformForEntity();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Model and draw

#define MAX_LBM_HEIGHT  480

#define BACKFACE_EPSILON	0.01

#include "dx_gl_model.h"

extern int c_brush_polys, c_alias_polys;
extern int r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;
extern entity_t *currententity;
extern model_t *currentmodel;
extern model_t	*r_worldmodel;

extern cplane_t frustum[4];

void R_BeginRegistration(char *model);
void R_EndRegistration(void);


// Current simplification on purpose: all world/mesh draw are using the same vertex layout
struct MeshVertexFormat
{
	float Position[3];
	float SurfaceUV[2];
	float LightmapUV[2];
	float ColorAlpha[4];
};

struct MeshRenderCommand
{
	enum class EType
	{
		DrawInstanced_Colored,
		DrawInstanced_ColoredSurface,
		DrawInstanced_LightmapSurface,
	};
	EType Type;

	bool bEnableAlphaBlending;

	RenderTexture* SurfaceTexture;
	RenderTexture* LightmapTexture;

	D3D_PRIMITIVE_TOPOLOGY Topology;

	XMFLOAT4X4 MeshWorldMatrix; // float4x4 but using XMFLOAT4X4 to not force alignement constraint (TODO implement aligned allocator)

	// Used by both DrawInstanced and DrawIndexedInstanced
	UINT InstanceCount;
	UINT StartInstanceLocation;

	// Used by DrawInstanced only
	UINT VertexCountPerInstance;
	UINT StartVertexLocation;

	// Used by DrawIndexedInstanced only
	UINT IndexCountPerInstance;
	UINT StartIndexLocation;
	UINT BaseVertexLocation;
};

#define VertexMemorySizeBytes	10 * 1024 * 1024
#define IndexMemorySizeBytes	1 * 1024 * 1024
#define MaxCommandCount			8912
struct MeshRenderer
{
public:
	MeshRenderer();
	~MeshRenderer();

	void StartRecording();

	void StopRecording();

	void StartCommand(
		MeshRenderCommand::EType Type, 
		float4x4 MeshWorldMatrix = XMMatrixIdentity(), 
		RenderTexture* SurfaceTexture = nullptr,
		RenderTexture* LightmapTexture = nullptr,
		D3D_PRIMITIVE_TOPOLOGY Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	void SetCurrentCommandUseAlphaBlending();

	void AppendVertex(MeshVertexFormat& NewVertex);

	// TODO AppendIndex

	void EndCommand();

	void ExecuteRenderCommands();


private:
	RenderBufferGenericDynamic* MeshVertexRenderBuffer = nullptr;
	RenderBufferGenericDynamic* MeshIndexRenderBuffer = nullptr;

	bool bRecordingStarted = false;
	bool bCommandStarted = false;
	uint AllocatedVertexBytes;
	uint AllocatedIndexBytes;
	uint RecordedVertexCount;
	uint RecordedIndexCount;

	MeshVertexFormat* MeshVertexMemory;
	uint* MeshIndexMemory;

	MeshRenderCommand* RenderCommands;
	uint RecordedRenderCommandCount;
	MeshRenderCommand* CurrentCommand;
};

extern MeshRenderer* gMeshRenderer;



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
extern cvar_t *r_nocull;
extern cvar_t *r_speeds;
extern cvar_t *r_lightlevel;
extern cvar_t *r_drawentities;
extern cvar_t *r_lefthand;

extern cvar_t *gl_lockpvs;
extern cvar_t *gl_flashblend;
extern cvar_t *gl_modulate;
extern cvar_t *gl_monolightmap;
extern cvar_t *gl_polyblend;
extern cvar_t *gl_particle_size;
extern cvar_t *gl_dynamic;

extern int r_visframecount;
extern int r_framecount;
extern int r_dlightframecount;

extern int registration_sequence;
extern int c_visible_lightmaps;
extern int c_visible_textures;

struct ViewData
{
	vec3_t forward;
	vec3_t right;
	vec3_t up;

	float4x4 ViewMatrix;
	float4x4 ViewMatrixInv;
	float4x4 ProjectionMatrix;
	float4x4 ProjectionMatrixInv;
	float4x4 ViewProjectionMatrix;
	float4x4 ViewProjectionMatrixInv;
};
// ignoring VR views
ViewData GetViewData();

extern RenderTexture* DepthTexture;

