
#include "dx_local.h"


static char		skyname[MAX_QPATH];
static float	skyrotate;
static vec3_t	skyaxis;
static image_t*	sky_images[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

static char*	suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };


void SkyRegisterTexture(char* name, float rotate, vec3_t axis)
{
	int		i;
	char	pathname[MAX_QPATH];

	strncpy(skyname, name, sizeof(skyname) - 1);
	skyrotate = rotate;
	VectorCopy(axis, skyaxis);

	for (i = 0; i < 6; i++)
	{
		Com_sprintf(pathname, sizeof(pathname), "env/%s%s.tga", skyname, suf[i]);

		sky_images[i] = FindImage(pathname, it_sky);
		if (!sky_images[i])
			sky_images[i] = r_notexture;
	}
}

void SkyUnregisterTexture()
{
	sky_images[0] = sky_images[1] = sky_images[2] = sky_images[3] = sky_images[4] = sky_images[5] = nullptr;
}

void SkyRender()
{
	if (sky_images[0])
	{
		DrawImageCall dic;
		dic.Image = sky_images[0];
		dic.Type = DrawImageCallType::Draw_Pic;
		dic.x = 100;
		dic.y = 100;
		dic.w = 256;
		dic.h = 256;
		AddDrawImage(dic);
	}
}


