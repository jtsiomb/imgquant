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
