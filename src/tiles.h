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
#ifndef TILES_H_
#define TILES_H_

#include "image.h"

struct tilemap {
	int width, height;
	int *map;
};

int img2tiles(struct tilemap *tmap, struct image *img, int tw, int th, int dedup);
int dump_tilemap(struct tilemap *tmap, const char *fname);

#endif	/* TILES_H_ */
