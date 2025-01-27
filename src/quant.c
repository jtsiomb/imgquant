#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include "image.h"

#define NUM_LEVELS	8

struct octnode;

struct octree {
	struct octnode *root;
	struct octnode *redlist[NUM_LEVELS];
	int redlev;
	int nleaves, maxcol;
};

struct octnode {
	int lvl;
	struct octree *tree;
	int r, g, b, nref;
	int palidx;
	int nsub, leaf;
	struct octnode *sub[8];
	struct octnode *next;
};


static void init_octree(struct octree *tree, int maxcol);
static void destroy_octree(struct octree *tree);

static struct octnode *alloc_node(struct octree *tree, int lvl);
static void free_node(struct octnode *n);
static void free_tree(struct octnode *n);

static void add_color(struct octree *tree, int r, int g, int b, int nref);
static void reduce_colors(struct octree *tree);
static int assign_colors(struct octnode *n, int next, struct cmapent *cmap);
static int lookup_color(struct octree *tree, int r, int g, int b);
static int subidx(int bit, int r, int g, int b);
static void print_tree(struct octnode *n, int lvl);


#define CLAMP(x, a, b)	((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))
static void add_error(struct image *dest, int x, int y, int *err, int s, int *acc)
{
	int r, g, b;
	unsigned int pixel[3];

	if(s) {
		r = s * err[0] >> 4;
		g = s * err[1] >> 4;
		b = s * err[2] >> 4;
		acc[0] += r;
		acc[1] += g;
		acc[2] += b;
	} else {
		r = err[0];
		g = err[1];
		b = err[2];
	}
	get_pixel_rgb(dest, x, y, pixel);
	r += pixel[0];
	g += pixel[1];
	b += pixel[2];
	pixel[0] = CLAMP(r, 0, 255);
	pixel[1] = CLAMP(g, 0, 255);
	pixel[2] = CLAMP(b, 0, 255);
	put_pixel_rgb(dest, x, y, pixel);
}


int quantize_image(struct image *img, int maxcol, enum dither dither,
		int shade_levels, int *shade_lut)
{
	int i, j, cidx;
	unsigned int rgb[3];
	struct octree tree;
	struct image newimg = *img;
	int err[3], acc[3];

	if(maxcol < 2 || maxcol > 256) {
		return -1;
	}
	if(shade_lut && shade_levels <= 1) {
		return -1;
	}

	if(img->bpp > 8) {
		newimg.bpp = maxcol > 16 ? 8 : 4;
		newimg.nchan = 1;
		newimg.scansz = newimg.width * newimg.bpp / 8;
		newimg.pitch = newimg.scansz;
	}

	init_octree(&tree, maxcol);

	for(i=0; i<img->height; i++) {
		for(j=0; j<img->width; j++) {
			get_pixel_rgb(img, j, i, rgb);
			add_color(&tree, rgb[0], rgb[1], rgb[2], 1024);

			while(tree.nleaves > maxcol) {
				reduce_colors(&tree);
			}
		}
	}

	if(shade_lut) {
		/* temporary colormap to add ramps */
		newimg.cmap_ncolors = assign_colors(tree.root, 0, newimg.cmap);

		for(i=0; i<img->cmap_ncolors; i++) {
			for(j=0; j<shade_levels - 1; j++) {
				rgb[0] = img->cmap[i].r * j / (shade_levels - 1);
				rgb[1] = img->cmap[i].g * j / (shade_levels - 1);
				rgb[2] = img->cmap[i].b * j / (shade_levels - 1);
				add_color(&tree, rgb[0], rgb[1], rgb[2], 1);

				while(tree.nleaves > maxcol) {
					reduce_colors(&tree);
				}
			}
		}
	}

	/* use created octree to generate the palette */
	newimg.cmap_ncolors = assign_colors(tree.root, 0, newimg.cmap);

	/* replace image pixels */
	for(i=0; i<img->height; i++) {
		for(j=0; j<img->width; j++) {
			get_pixel_rgb(img, j, i, rgb);
			cidx = lookup_color(&tree, rgb[0], rgb[1], rgb[2]);
			assert(cidx >= 0 && cidx < maxcol);
			put_pixel(&newimg, j, i, cidx);

			switch(dither) {
			case DITHER_FLOYD_STEINBERG:
				err[0] = (int)rgb[0] - (int)newimg.cmap[cidx].r;
				err[1] = (int)rgb[1] - (int)newimg.cmap[cidx].g;
				err[2] = (int)rgb[2] - (int)newimg.cmap[cidx].b;
				acc[0] = acc[1] = acc[2] = 0;
				if(j < img->width - 1) {
					add_error(img, j + 1, i, err, 7, acc);
				}
				if(i < img->height - 1) {
					if(j > 0) {
						add_error(img, j - 1, i + 1, err, 3, acc);
					}
					add_error(img, j, i + 1, err, 5, acc);
					if(j < img->width - 1) {
						err[0] -= acc[0];
						err[1] -= acc[1];
						err[2] -= acc[2];
						add_error(img, j + 1, i + 1, err, 0, 0);
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if(shade_lut) {
		/* populate shade_lut based on the new palette, can't generate levels only
		 * for the original colors, because the palette entries will have changed
		 * and moved around.
		 */
		for(i=0; i<newimg.cmap_ncolors; i++) {
			for(j=0; j<shade_levels; j++) {
				rgb[0] = newimg.cmap[i].r * j / (shade_levels - 1);
				rgb[1] = newimg.cmap[i].g * j / (shade_levels - 1);
				rgb[2] = newimg.cmap[i].b * j / (shade_levels - 1);
				*shade_lut++ = lookup_color(&tree, rgb[0], rgb[1], rgb[2]);
			}
		}
		for(i=0; i<(maxcol - newimg.cmap_ncolors) * shade_levels; i++) {
			*shade_lut++ = maxcol - 1;
		}
	}

	*img = newimg;

	destroy_octree(&tree);
	return 0;
}

static void init_octree(struct octree *tree, int maxcol)
{
	memset(tree, 0, sizeof *tree);

	tree->redlev = NUM_LEVELS - 1;
	tree->maxcol = maxcol;

	tree->root = alloc_node(tree, 0);
}

static void destroy_octree(struct octree *tree)
{
	free_tree(tree->root);
}

static struct octnode *alloc_node(struct octree *tree, int lvl)
{
	struct octnode *n;

	if(!(n = calloc(1, sizeof *n))) {
		perror("failed to allocate octree node");
		abort();
	}

	n->lvl = lvl;
	n->tree = tree;
	n->palidx = -1;

	if(lvl < tree->redlev) {
		n->next = tree->redlist[lvl];
		tree->redlist[lvl] = n;
	} else {
		n->leaf = 1;
		tree->nleaves++;
	}
	return n;
}

static void free_node(struct octnode *n)
{
	struct octnode *prev, dummy;

	dummy.next = n->tree->redlist[n->lvl];
	prev = &dummy;
	while(prev->next) {
		if(prev->next == n) {
			prev->next = n->next;
			break;
		}
		prev = prev->next;
	}
	n->tree->redlist[n->lvl] = dummy.next;

	if(n->leaf) {
		n->tree->nleaves--;
		assert(n->tree->nleaves >= 0);
	}
	free(n);
}

static void free_tree(struct octnode *n)
{
	int i;

	if(!n) return;

	for(i=0; i<8; i++) {
		free_tree(n->sub[i]);
	}
	free_node(n);
}

static void add_color(struct octree *tree, int r, int g, int b, int nref)
{
	int i, idx, rr, gg, bb;
	struct octnode *n;

	rr = r * nref;
	gg = g * nref;
	bb = b * nref;

	n = tree->root;
	n->r += rr;
	n->g += gg;
	n->b += bb;
	n->nref += nref;

	for(i=0; i<NUM_LEVELS; i++) {
		if(n->leaf) break;

		idx = subidx(i, r, g, b);

		if(!n->sub[idx]) {
			n->sub[idx] = alloc_node(tree, i + 1);
		}
		n->nsub++;
		n = n->sub[idx];

		n->r += rr;
		n->g += gg;
		n->b += bb;
		n->nref += nref;
	}
}

static struct octnode *get_reducible(struct octree *tree)
{
	int best_nref;
	struct octnode dummy, *n, *prev, *best_prev, *best = 0;

	while(tree->redlev >= 0) {
		best_nref = INT_MAX;
		best = 0;
		dummy.next = tree->redlist[tree->redlev];
		prev = &dummy;
		while(prev->next) {
			n = prev->next;
			if(n->nref < best_nref) {
				best = n;
				best_nref = n->nref;
				best_prev = prev;
			}
			prev = prev->next;
		}
		if(best) {
			best_prev->next = best->next;
			tree->redlist[tree->redlev] = dummy.next;
			break;
		}
		tree->redlev--;
	}

	return best;
}

static void reduce_colors(struct octree *tree)
{
	int i;
	struct octnode *n;

	if(!(n = get_reducible(tree))) {
		fprintf(stderr, "warning: no reducible nodes!\n");
		return;
	}
	for(i=0; i<8; i++) {
		if(n->sub[i]) {
			free_node(n->sub[i]);
			n->sub[i] = 0;
		}
	}
	n->leaf = 1;
	tree->nleaves++;
}

static int assign_colors(struct octnode *n, int next, struct cmapent *cmap)
{
	int i;

	if(!n) return next;

	if(n->leaf) {
		assert(next < n->tree->maxcol);
		assert(n->nref);
		cmap[next].r = n->r / n->nref;
		cmap[next].g = n->g / n->nref;
		cmap[next].b = n->b / n->nref;
		n->palidx = next;
		return next + 1;
	}

	for(i=0; i<8; i++) {
		next = assign_colors(n->sub[i], next, cmap);
	}
	return next;
}

static int lookup_color(struct octree *tree, int r, int g, int b)
{
	int i, j, idx;
	struct octnode *n;

	n = tree->root;
	for(i=0; i<NUM_LEVELS; i++) {
		if(n->leaf) {
			assert(n->palidx >= 0);
			return n->palidx;
		}

		idx = subidx(i, r, g, b);
		for(j=0; j<8; j++) {
			if(n->sub[idx]) break;
			idx = (idx + 1) & 7;
		}

		assert(n->sub[idx]);
		n = n->sub[idx];
	}

	fprintf(stderr, "lookup_color(%d, %d, %d) failed!\n", r, g, b);
	abort();
	return -1;
}

static int subidx(int bit, int r, int g, int b)
{
	assert(bit >= 0 && bit < NUM_LEVELS);
	bit = NUM_LEVELS - 1 - bit;
	return ((r >> bit) & 1) | ((g >> (bit - 1)) & 2) | ((b >> (bit - 2)) & 4);
}

static void print_tree(struct octnode *n, int lvl)
{
	int i;
	char ptrbuf[32], *p;

	if(!n) return;

	for(i=0; i<lvl; i++) {
		fputs("|  ", stdout);
	}

	sprintf(ptrbuf, "%p", (void*)n);
	p = ptrbuf + strlen(ptrbuf) - 4;

	if(n->nref) {
		printf("+-(%d) %s: <%d %d %d> #%d", n->lvl, p, n->r / n->nref, n->g / n->nref,
				n->b / n->nref, n->nref);
	} else {
		printf("+-(%d) %s: <- - -> #0", n->lvl, p);
	}
	if(n->palidx >= 0) printf(" [%d]", n->palidx);
	if(n->leaf) printf(" LEAF");
	putchar('\n');

	for(i=0; i<8; i++) {
		print_tree(n->sub[i], lvl + 1);
	}
}

