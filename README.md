imgquant
========

About
-----
imgquant is a tool for quantizing image colors and perform a number of
operations useful on retro platforms:

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

Usage examples
--------------
Convert true color pre-rendered tileset to 128 colors, save the result as
palettized PNG (`out.png`), and also generate a 16-level shade LUT and save it
as a separate file (`out.slut`):

    ./imgquant tileset.png -P -o out.png -C 128 -s 16 -os out.slut
               \_________/  ^            \____/ \___/  \_________/
               input file   |      max # colors   |    shade LUT output file
                   output as PNG      generate 16 level shade LUT
