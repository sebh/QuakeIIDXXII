
#include "dx_local.h"



//Finds or loads the given image
image_t	*FindImage(char *name, imagetype_t type)
{
	return nullptr;
}



image_t	*Draw_FindPic(char *name)
{
	image_t *Image;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf(fullname, sizeof(fullname), "pics/%s.pcx", name);
		Image = FindImage(fullname, it_pic);
	}
	else
		Image = FindImage(name + 1, it_pic);

	return Image;
}


