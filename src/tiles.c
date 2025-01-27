#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tiles.h"
#include "image.h"

static int matchtile(struct image *img, int toffs, int th);

int img2tiles(struct tilemap *tmap, struct image *img, int tw, int th, int dedup)
{
	int i, j, x, y, tx, ty, tileoffs, xtiles, ytiles, ntiles, tileno, tid;
	struct image orig;
	unsigned int pix;

	if(alloc_image(&orig, img->width, img->height, img->bpp) == -1) {
		fprintf(stderr, "img2tiles: failed to allocate temporary image\n");
		return -1;
	}
	memcpy(orig.pixels, img->pixels, img->scansz * img->height);

	xtiles = (img->width + tw - 1) / tw;
	ytiles = (img->height + th - 1) / th;
	ntiles = xtiles * ytiles;

	img->width = tw;
	img->height = ntiles * th;
	img->pitch = img->scansz = tw * img->bpp / 8;

	if(tmap) {
		tmap->width = xtiles;
		tmap->height = ytiles;
		if(!(tmap->map = malloc(ntiles * sizeof *tmap->map))) {
			fprintf(stderr, "failed to allocate tilemap\n");
			free(orig.pixels);
			return -1;
		}
	}

	tileno = 0;
	tileoffs = 0;
	y = 0;
	for(i=0; i<ytiles; i++) {
		x = 0;
		for(j=0; j<xtiles; j++) {
			for(ty=0; ty<th; ty++) {
				for(tx=0; tx<tw; tx++) {
					pix = get_pixel(&orig, x + tx, y + ty);
					put_pixel(img, tx, ty + tileoffs, pix);
				}
			}

			if(dedup) {
				if((tid = matchtile(img, tileoffs, th)) == -1) {
					if(tmap) {
						tmap->map[tileno++] = tileoffs / th;
					}
					tileoffs += th;	/* destination Y offset, inc by th for every tile */
				} else {
					if(tmap) {
						tmap->map[tileno++] = tid;
					}
				}
			} else {
				tileoffs += th;	/* destination Y offset, inc by th for every tile */
			}
			x += tw;
		}
		y += th;
	}

	if(dedup) {
		putchar('\n');
		img->height = tileoffs;
	}

	free(orig.pixels);
	return 0;
}

static int matchtile(struct image *img, int toffs, int th)
{
	int i, tilesz;
	int ntiles = toffs / th;
	unsigned char *pa, *pb;

	tilesz = img->pitch * th;
	pa = (unsigned char*)img->pixels;
	pb = (unsigned char*)img->pixels + toffs * img->pitch;

	for(i=0; i<ntiles; i++) {
		if(memcmp(pa, pb, tilesz) == 0) {
			return i;
		}
		pa += tilesz;
	}

	return -1;
}

int dump_tilemap(struct tilemap *tmap, const char *fname)
{
	FILE *fp;
	int i, sz = tmap->width * tmap->height;
	uint16_t id;

	if(sz <= 0) return -1;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "dump_tilemap: failed to open %s for writing\n", fname);
		return -1;
	}

	for(i=0; i<sz; i++) {
		/* XXX dump in 16bit big endian for the megadrive */
		id = (tmap->map[i] << 8) | (tmap->map[i] >> 8);
		fwrite(&id, sizeof id, 1, fp);
	}

	fclose(fp);
	return 0;
}
