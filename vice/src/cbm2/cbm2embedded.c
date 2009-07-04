/*
 * cbm2embedded.c - Code for embedding cbm2 data files.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#ifdef USE_EMBEDDED
#include <string.h>
#include <stdio.h>

#include "embedded.h"
#include "machine.h"

#include "cbm2basic128.h"
#include "cbm2basic256.h"
#include "cbm2basic500.h"
#include "cbm2chargen500.h"
#include "cbm2chargen600.h"
#include "cbm2chargen700.h"
#include "cbm2kernal.h"
#include "cbm2kernal500.h"

static embedded_t cbm2files[] = {
  { "basic.128", 0x4000, 0x4000, 0x4000, cbm2basic128_embedded },
  { "basic.256", 0x4000, 0x4000, 0x4000, cbm2basic256_embedded },
  { "basic.500", 0x4000, 0x4000, 0x4000, cbm2basic500_embedded },
  { "chargen.500", 0x1000, 0x1000, 0x1000, cbm2chargen500_embedded },
  { "chargen.600", 0x1000, 0x1000, 0x1000, cbm2chargen600_embedded },
  { "chargen.700", 0x1000, 0x1000, 0x1000, cbm2chargen700_embedded },
  { "kernal", 0x2000, 0x2000, 0x2000, cbm2kernal_embedded },
  { "kernal.500", 0x2000, 0x2000, 0x2000, cbm2kernal500_embedded },
  { NULL }
};

static size_t embedded_match_file(const char *name, BYTE *dest, int minsize, int maxsize, embedded_t *emb)
{
    int i = 0;

    while (emb[i].name != NULL) {
        if (!strcmp(name, emb[i].name) && minsize == emb[i].minsize && maxsize == emb[i].maxsize) {
            if (emb[i].esrc != NULL) {
                if (emb[i].size != minsize) {
                    memcpy(dest, emb[i].esrc, maxsize);
                } else {
                    memcpy(dest + maxsize - minsize, emb[i].esrc, minsize);
                }
            }
            return emb[i].size;
        }
        i++;
    }
    return 0;
}

size_t embedded_check_file(const char *name, BYTE *dest, int minsize, int maxsize)
{
    size_t retval;

    if ((retval = embedded_check_extra(name, dest, minsize, maxsize)) != 0) {
        return retval;
    }

    if ((retval = embedded_match_file(name, dest, minsize,maxsize, cbm2files)) != 0) {
        return retval;
    }
    return 0;
}
#endif
