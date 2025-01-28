imgquant
========

About
-----
imgquant is a tool for image color quantization, and other operations useful
when hacking for retro platforms:

 - color quantization to an arbitrary colormap size
 - colormap generation with optional shade LUT
 - slicing into tiles with optional tile deduplication
 - creation of tilemaps to reconstruct image from deduplicated tiles
 - output as PNG or raw binary
 - conversion to 15bit 555 RGB
 - optimize colors for the Gameboy Advance screen
 - swap nibbles for 16 color images

License
-------
Copyright (C) 2021-2025  John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software, feel free to use, modify and/or redistribute it
under the terms of the GNU General Public License v3, or at your option any
later version published by the Free Software Foundation. See COPYING for
details.

Build
-----
To build imgquant, you need to have libpng (http://www.libpng.org) installed, which in turn depends on
zlib (http://zlib.net).

When you have satisfied the dependencies, just type `make` to build, and
`make install` to install.

The default installation prefix is `/usr/local`; if you want to change it,
just modify the first line of the `Makefile`.

Usage examples
--------------
Convert true color pre-rendered tileset to 128 colors, save the result as
palettized PNG (`out.png`), and also generate a 16-level shade LUT and save it
as a separate file (`out.slut`):

    ./imgquant tileset.png -P -o out.png -C 128 -s 16 -os out.slut
               \_________/  ^            \____/ \___/  \_________/
               input file   |      max # colors   |    shade LUT output file
                   output as PNG      generate 16 level shade LUT

Slice an image into 8x8 tiles, deduplicate to output only unique tiles to a raw
output image (`out.img`) and its colormap (`out.pal`), and construct a tilemap
that references these tiles to recreate the original image (`out.tmap`):

    ./imgquant input.png -T 8x8 -D -o out.img -oc out.pal -om out.tmap
                         \____/  ^            \_________/ \__________/
          slice into 8x8 tiles   |       output colormap   output tilemap
                         deduplicate tiles
