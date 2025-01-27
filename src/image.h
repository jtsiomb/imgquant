/*
imgquant - image processing tool for retro platform graphics hacking
Copyright (C) 2021-2025  John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef IMAGE_H_
#define IMAGE_H_

#include <stdio.h>

struct cmapent {
	unsigned char r, g, b;
};

struct image {
	int width, height;
	int bpp;
	int nchan;
	int scansz;	/* scanline size in bytes */
	int pitch;	/* bytes from one scanline to the next */
	int cmap_ncolors;
	struct cmapent cmap[256];
	unsigned char *pixels;
};

enum dither {
	DITHER_NONE,
	DITHER_FLOYD_STEINBERG
};

int alloc_image(struct image *img, int x, int y, int bpp);
int load_image(struct image *img, const char *fname);
int save_image(struct image *img, const char *fname);
int save_image_file(struct image *img, FILE *fp);

int cmp_image(struct image *a, struct image *b);

void blit(struct image *src, int sx, int sy, int w, int h, struct image *dst, int dx, int dy);
void overlay_key(struct image *src, unsigned int key, struct image *dst);

unsigned int get_pixel(struct image *img, int x, int y);
unsigned int get_pixel_rgb(struct image *img, int x, int y, unsigned int *rgb);
void put_pixel(struct image *img, int x, int y, unsigned int pix);
void put_pixel_rgb(struct image *img, int x, int y, unsigned int *rgb);

int quantize_image(struct image *img, int maxcol, enum dither dither,
		int shade_levels, int *shade_lut);

#endif	/* IMAGE_H_ */
