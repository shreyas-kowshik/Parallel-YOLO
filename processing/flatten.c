#include<stdlib.h>
#include<stdio.h>
#include<png.h>

int width, height;
png_byte color_type;
float *image;
png_byte bit_depth;
png_bytep *row_pointers = NULL;

void load(string filename) 
{
	FILE *fp = fopen(filename, "rb");
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();
	png_infop info = png_create_info_struct(png);
	if(!info) abort();
	if(setjmp(png_jmpbuf(png))) abort();
	png_init_io(png, fp);
	png_read_info(png, info);
	width      = png_get_image_width(png, info);
	height     = png_get_image_height(png, info);
	color_type = png_get_color_type(png, info);
	bit_depth  = png_get_bit_depth(png, info);
	if(bit_depth == 16) png_set_strip_16(png);
	if(color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
	if(png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
	if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE) png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png);
	png_read_update_info(png, info);
	if (row_pointers) abort();
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	png_read_image(png, row_pointers);
	fclose(fp);
	png_destroy_read_struct(&png, &info, NULL);
}

float* flatten(string filename)
{
	int i, j, x, y;
	load(filename);

	image = malloc(3 * height * width * sizeof(int*));
	int stride = height * width;
	for(y = 0; y < height; y++)
	{
		png_bytep row = row_pointers[y];
		for(x = 0; x < width; x++) 
		{
			png_bytep px = &(row[x * 4]);
			image[y * width + x]=px[0];
			image[y * width + x + stride] = px[1];
			image[y * width + x + 2 * stride] = px[2];
		}
	}
	
	return image;
}