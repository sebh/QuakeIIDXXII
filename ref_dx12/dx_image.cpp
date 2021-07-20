
#include "dx_local.h"

typedef std::map<std::string, image_t*> ImageMapType;
static ImageMapType ImageMap;

#define D_MAX_IMAGE_COUNT 2048

static uint AllocatedImageCount = 0;
static image_t Images[D_MAX_IMAGE_COUNT];

static bool bImageInitialised = false;
static image_t* r_notexture;

/*
==============
LoadPCX
==============
*/
void LoadPCX(char *filename, byte **pic, byte **palette, int *width, int *height)
{
	byte	*raw;
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	byte	*out, *pix;

	*pic = NULL;
	*palette = NULL;

	//
	// load the file
	//
	len = ri.FS_LoadFile(filename, (void **)&raw);
	if (!raw)
	{
		ri.Con_Printf(PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		return;
	}

	//
	// parse the PCX file
	//
	pcx = (pcx_t *)raw;

	pcx->xmin = LittleShort(pcx->xmin);
	pcx->ymin = LittleShort(pcx->ymin);
	pcx->xmax = LittleShort(pcx->xmax);
	pcx->ymax = LittleShort(pcx->ymax);
	pcx->hres = LittleShort(pcx->hres);
	pcx->vres = LittleShort(pcx->vres);
	pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
	pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		ri.Con_Printf(PRINT_ALL, "Bad pcx file %s\n", filename);
		return;
	}

	out = (byte*) malloc((pcx->ymax + 1) * (pcx->xmax + 1));

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = (byte*) malloc(768);
		memcpy(*palette, (byte *)pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax + 1;
	if (height)
		*height = pcx->ymax + 1;

	for (y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1)
	{
		for (x = 0; x <= pcx->xmax; )
		{
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while (runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if (raw - (byte *)pcx > len)
	{
		ri.Con_Printf(PRINT_DEVELOPER, "PCX file %s was malformed", filename);
		free(*pic);
		*pic = NULL;
	}

	ri.FS_FreeFile(pcx);
}


/*
=============
LoadTGA
=============
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

void LoadTGA(char *name, byte **pic, int *width, int *height)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;
	byte tmp[2];

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile(name, (void **)&buffer);
	if (!buffer)
	{
		ri.Con_Printf(PRINT_DEVELOPER, "Bad tga file %s\n", name);
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;

	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort(*((short *)tmp));
	buf_p += 2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort(*((short *)tmp));
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort(*((short *)buf_p));
	buf_p += 2;
	targa_header.y_origin = LittleShort(*((short *)buf_p));
	buf_p += 2;
	targa_header.width = LittleShort(*((short *)buf_p));
	buf_p += 2;
	targa_header.height = LittleShort(*((short *)buf_p));
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type != 2
		&& targa_header.image_type != 10)
		ri.Sys_Error(ERR_DROP, "LoadTGA: Only type 2 and 10 targa RGB images supported\n");

	if (targa_header.colormap_type != 0
		|| (targa_header.pixel_size != 32 && targa_header.pixel_size != 24))
		ri.Sys_Error(ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = (byte*) malloc(numPixels * 4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment

	if (targa_header.image_type == 2) {  // Uncompressed, RGB images
		for (row = rows - 1; row >= 0; row--) {
			pixbuf = targa_rgba + row * columns * 4;
			for (column = 0; column < columns; column++) {
				unsigned char red, green, blue, alphabyte;
				switch (targa_header.pixel_size) {
				case 24:

					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				}
			}
		}
	}
	else if (targa_header.image_type == 10) {   // Runlength encoded RGB images
		unsigned char red, green, blue, alphabyte, packetHeader, packetSize, j;
		for (row = rows - 1; row >= 0; row--) {
			pixbuf = targa_rgba + row * columns * 4;
			for (column = 0; column < columns; ) {
				packetHeader = *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						break;
					}

					for (j = 0; j < packetSize; j++) {
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						column++;
						if (column == columns) { // run spans across rows
							column = 0;
							if (row > 0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
				else {                            // non run-length packet
					for (j = 0; j < packetSize; j++) {
						switch (targa_header.pixel_size) {
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						}
						column++;
						if (column == columns) { // pixel packet run spans across rows
							column = 0;
							if (row > 0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
			}
		breakOut:;
		}
	}

	ri.FS_FreeFile(buffer);
}

/*
================
GL_LoadPic

This is also used as an entry point for the generated r_notexture
================
*/
image_t *GL_LoadPic(char *name, byte *pic, int width, int height, imagetype_t type, int bits)
{
	image_t*	Image = nullptr;

	ATLASSERT(AllocatedImageCount < D_MAX_IMAGE_COUNT);
	if (AllocatedImageCount < D_MAX_IMAGE_COUNT)
	{
		uint ImageIdx = AllocatedImageCount++;
		Image = &Images[ImageIdx];
		ImageMap[std::string(name)] = Image;

		// TODO
		//Image->texnum = TEXNUM_IMAGES + (image - gltextures);

		// Copy the data for deferred upload
		const int PicSize = width * height * (bits / 8);
		Image->pic = new byte[PicSize];
		memcpy_s(Image->pic, PicSize, pic, PicSize);
		Image->bits = bits;
		Image->has_alpha = true;
		Image->uploaded = false;
		Image->RenderTexture = nullptr;

		// TODO
		//if (bits == 8)
		//	Image->has_alpha = GL_Upload8(pic, width, height, (image->type != it_pic && image->type != it_sky), image->type == it_sky);
		//else
		//	Image->has_alpha = GL_Upload32((unsigned *)pic, width, height, (image->type != it_pic && image->type != it_sky));

		// TODO
		//Image->upload_width = upload_width;		// after power of 2 and scales
		//Image->upload_height = upload_height;		// Set from GL_Upload32 or GL_Upload8
		//Image->paletted = uploaded_paletted;
		Image->upload_width = width;
		Image->upload_height = height;
		Image->width = width;
		Image->height = height;
		Image->type = type;

		if (strlen(name) >= sizeof(Image->name))
			ri.Sys_Error(ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
		strcpy(Image->name, name);
	}

	return Image;

/*	image_t		*image;
	int			i;

	std::string NameStr = std::string(name);

	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			ri.Sys_Error(ERR_DROP, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	if (strlen(name) >= sizeof(image->name))
		ri.Sys_Error(ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
	strcpy(image->name, name);
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->type = type;

	if (type == it_skin && bits == 8)
		R_FloodFillSkin(pic, width, height);

	// load little pics into the scrap
	if (image->type == it_pic && bits == 8
		&& image->width < 64 && image->height < 64)
	{
		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock(image->width, image->height, &x, &y);
		if (texnum == -1)
			goto nonscrap;
		scrap_dirty = true;

		// copy the texels into the scrap block
		k = 0;
		for (i = 0; i < image->height; i++)
			for (j = 0; j < image->width; j++, k++)
				scrap_texels[texnum][(y + i)*BLOCK_WIDTH + x + j] = pic[k];
		image->texnum = TEXNUM_SCRAPS + texnum;
		image->scrap = true;
		image->has_alpha = true;
		image->sl = (x + 0.01) / (float)BLOCK_WIDTH;
		image->sh = (x + image->width - 0.01) / (float)BLOCK_WIDTH;
		image->tl = (y + 0.01) / (float)BLOCK_WIDTH;
		image->th = (y + image->height - 0.01) / (float)BLOCK_WIDTH;
	}
	else
	{
	nonscrap:
		image->scrap = false;
		image->texnum = TEXNUM_IMAGES + (image - gltextures);
		GL_Bind(image->texnum);
		if (bits == 8)
			image->has_alpha = GL_Upload8(pic, width, height, (image->type != it_pic && image->type != it_sky), image->type == it_sky);
		else
			image->has_alpha = GL_Upload32((unsigned *)pic, width, height, (image->type != it_pic && image->type != it_sky));
		image->upload_width = upload_width;		// after power of 2 and scales
		image->upload_height = upload_height;
		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;
	}

	return image;*/
}


/*
================
GL_LoadWal
================
*/
image_t *GL_LoadWal(char *name)
{
	miptex_t	*mt;
	int			width, height, ofs;
	image_t		*image;

	ri.FS_LoadFile(name, (void **)&mt);
	if (!mt)
	{
		ri.Con_Printf(PRINT_ALL, "GL_FindImage: can't load %s\n", name);
		ri.Sys_Error(ERR_DROP, "GL_FindImage: can't load %s\n", name);
		//return r_notexture;
		return nullptr;
	}

	width = LittleLong(mt->width);
	height = LittleLong(mt->height);
	ofs = LittleLong(mt->offsets[0]);

	image = GL_LoadPic(name, (byte *)mt + ofs, width, height, it_wall, 8);

	ri.FS_FreeFile((void *)mt);

	return image;
}


//Finds or loads the given image
image_t* FindImage(char *name, imagetype_t type)
{
	//return r_notexture;	// SEBH TEMP

	image_t	*image = nullptr;
	int		len;
	byte	*pic, *palette;
	int		width, height;

	if (!name)
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GL_FindImage: NULL name");
	len = strlen(name);
	if (len < 5)
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GL_FindImage: bad name: %s", name);

	std::string NameStr = std::string(name);

	// look for it
	ImageMapType::iterator it = ImageMap.find(NameStr);
	if (it != ImageMap.end())
	{
		return it->second;
	}

	//
	// load the pic from disk
	//
	pic = NULL;
	palette = NULL;
	if (!strcmp(name + len - 4, ".pcx"))
	{
		LoadPCX(name, &pic, &palette, &width, &height);
		if (!pic)
		{
			ri.Sys_Error(ERR_DROP, "GL_FindImage: can't load %s", name);
			return NULL;
		}
		image = GL_LoadPic(name, pic, width, height, type, 8);
	}
	else if (!strcmp(name + len - 4, ".wal"))
	{
		image = GL_LoadWal(name);
	}
	else if (!strcmp(name + len - 4, ".tga"))
	{
		LoadTGA(name, &pic, &width, &height);
		if (!pic)
		{
			ri.Sys_Error(ERR_DROP, "GL_FindImage: can't load %s", name);
			return NULL;
		}
		image = GL_LoadPic(name, pic, width, height, type, 32);
	}
	else
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GL_FindImage: bad extension on: %s", name);


	if (pic)
		free(pic);
	if (palette)
		free(palette);

	return image;
}



image_t* Draw_FindPic(char *name)
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






void UploadAllTextures()
{
	if (!bImageInitialised)
	{
		byte dottexture[8][8] =
		{
			{000,000,000,255,255,000,000,000},
			{000,128,255,255,255,255,128,000},
			{000,255,255,255,255,255,255,000},
			{255,255,255,255,255,255,255,255},
			{255,255,255,255,255,255,255,255},
			{000,255,255,255,255,255,255,000},
			{000,128,255,255,255,255,128,000},
			{000,000,000,255,255,000,000,000},
		};
		r_notexture = GL_LoadPic("***r_notexture***", (byte *)dottexture, 8, 8, it_wall, 8);

		bImageInitialised = true;
	}

	for (auto& it : ImageMap)
	{
		if (!it.second->uploaded)
		{
			if (it.second->bits != 8 && it.second->bits != 32)
			{
				ErrorExit("UploadAllTextures : Unsupported image format\n");
			}

			const int PicSizeBytes  = it.second->upload_width * it.second->upload_height * it.second->bits / 8;
			const int RowPitchBytes = it.second->upload_width * it.second->bits / 8;
			const int SlicePitchBytes = PicSizeBytes;

			it.second->RenderTexture = new RenderTexture(
				it.second->upload_width, it.second->upload_height,
				1, it.second->bits==8 ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM,
				D3D12_RESOURCE_FLAG_NONE,
				nullptr,
				PicSizeBytes, RowPitchBytes, SlicePitchBytes, it.second->pic);
			it.second->uploaded = true;
		}
	}
}

void UnloadAllTextures()
{
	for (auto& it : ImageMap)
	{
		delete [] it.second->pic;
		it.second->pic = nullptr;
		delete it.second->RenderTexture;
		it.second->RenderTexture = nullptr;
	}

	bImageInitialised = false;
}
