/* $Id$ */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "libemu.h"
#include "global.h"
#include "sprites.h"
#include "unknown/unknown.h"
#include "house.h"
#include "pool/house.h"
#include "string.h"
#include "file.h"
#include "tools.h"

csip32 *g_sprites = NULL;

/**
 * Initialize the sprites system.
 *
 * @init System_Init_Sprites
 */
void System_Init_Sprites()
{
	g_sprites = (csip32 *)&emu_get_memory8(0x2DCE, 0x0, 0x440);
}

/**
 * ??.
 *
 * @param sprite_csip The CSIP of the sprite to work on.
 * @param arg0A_csip ??.
 */
static void Sprites_Unknown_2BB6_000C(csip32 sprite_csip, csip32 arg0A_csip)
{
	uint8 *sprite;
	uint8 *arg0A;
	uint8 i;

	if (sprite_csip.csip == 0x0 || arg0A_csip.csip == 0x0) return;

	sprite = emu_get_memorycsip(sprite_csip);
	arg0A = emu_get_memorycsip(arg0A_csip);

	if ((*sprite & 0x1) == 0) return;

	sprite += 10;

	for (i = 0; i < 16; i++) {
		uint8 offset = *sprite;
		*sprite++ = arg0A[offset];
	}
}

/**
 * Loads the sprites.
 *
 * @param index The index of the list of sprite files to load.
 * @param memory The index of memory block where to store loaded sprites.
 * @param sprites The array where to store CSIP for each loaded sprite.
 */
void Sprites_Load(uint16 index, uint16 memory, csip32 *sprites)
{
#define M(x) x "\0"
	static const char *spriteFiles[3] = {
		M("MOUSE.SHP")    /*   0 -   6 */
		M("BTTN")         /*   7 -  11 */
		M("SHAPES.SHP")   /*  12 - 110 */
		M("UNITS2.SHP")   /* 111 - 150 */
		M("UNITS1.SHP")   /* 151 - 237 */
		M("UNITS.SHP")    /* 238 - 354 */
		M(""),
		M("MENTAT")       /*   0 -  13 */
		M("MENSHP%c.SHP") /*  14 -  28 */
		M(""),
		M("MOUSE.SHP")    /*   0 -   6 */
		M("BTTN")         /*   7 -  11 */
		M("SHAPES.SHP")   /*  12 - 110 */
		M("CHOAM")        /* 111 - 128 */
		M("")
	};
#undef M

	const char *files;
	csip32 memBlock;
	uint8 *buffer;
	uint16 i;

	emu_push(memory);
	emu_push(emu_cs); emu_push(0x0A4C); emu_cs = 0x252E; emu_Memory_GetBlock1();
	emu_sp += 2;
	memBlock.s.cs = emu_dx;
	memBlock.s.ip = emu_ax;
	buffer = emu_get_memorycsip(memBlock);

	files = spriteFiles[index];

	while (*files != '\0') {
		uint32 length;
		HouseInfo *hi;
		char *filename = (char *)g_global->variable_9939;

		hi = &g_houseInfo[(g_global->playerHouseID == HOUSE_INDEX_INVALID) ? HOUSE_ATREIDES : g_global->playerHouseID];

		sprintf(filename, files, *(char *)emu_get_memorycsip(hi->name));

		if (strchr(filename, '.') == NULL) {
			filename = String_GenerateFilename(filename);
		}

		length = File_ReadBlockFile(filename, buffer, 0xFDE8);

		for (i = 0; i < *(uint16 *)buffer; i++) {
			*sprites++ = Sprites_GetCSIP(memBlock, i);
		}

		files += strlen(files) + 1;
		memBlock.csip += length;
		buffer += length;
	}

	switch (index) {
		case 0:
			for (i = 7; i < 12; i++) Sprites_Unknown_2BB6_000C(g_sprites[i], g_global->variable_3C42);
			break;

		case 2:
			for (i = 111; i < 129; i++) Sprites_Unknown_2BB6_000C(g_sprites[i], g_global->variable_3C42);
			break;

		default: break;
	}
}

/**
 * Gets the CSIP of the given sprite inside the given buffer.
 *
 * @param buffer_csip The CSIP of the buffer containing sprites.
 * @param index The index of the sprite to get the CSIP for.
 * @return The CSIP.
 */
csip32 Sprites_GetCSIP(csip32 buffer_csip, uint16 index)
{
	csip32 ret;
	uint16 *buffer;

	ret.csip = 0;

	if (buffer_csip.s.cs == 0) return ret;

	buffer = (uint16 *)emu_get_memorycsip(buffer_csip);

	if (*buffer++ <= index) return ret;

	buffer += 2 * index;

	if (*buffer == 0) return ret;

	buffer_csip.s.cs += buffer[1];
	buffer_csip.s.ip += buffer[0] + 2;

	return Tools_GetSmallestIP(buffer_csip);
}