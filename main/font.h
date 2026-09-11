/*
 * font.h - Minimal 5x7 glyph definitions
 *
 * Column-major layout: each glyph is GLYPH_WIDTH bytes, one byte per column.
 * Within each column byte, bits 0-6 run from top to bottom; bit 7 is padding.
 */

#ifndef FONT_H_GUARD
#define FONT_H_GUARD

#include <stdint.h>

#define GLYPH_WIDTH    5  /* Columns per glyph */
#define GLYPH_SPACING  6  /* Five columns plus one blank column */

const uint8_t *font_get_glyph(char character);

#define FONT_GLYPH_A  font_get_glyph('A')
#define FONT_GLYPH_B  font_get_glyph('B')
#define FONT_GLYPH_C  font_get_glyph('C')
#define FONT_GLYPH_D  font_get_glyph('D')
#define FONT_GLYPH_E  font_get_glyph('E')
#define FONT_GLYPH_F  font_get_glyph('F')
#define FONT_GLYPH_G  font_get_glyph('G')
#define FONT_GLYPH_H  font_get_glyph('H')
#define FONT_GLYPH_I  font_get_glyph('I')
#define FONT_GLYPH_J  font_get_glyph('J')
#define FONT_GLYPH_K  font_get_glyph('K')
#define FONT_GLYPH_L  font_get_glyph('L')
#define FONT_GLYPH_M  font_get_glyph('M')
#define FONT_GLYPH_N  font_get_glyph('N')
#define FONT_GLYPH_O  font_get_glyph('O')
#define FONT_GLYPH_P  font_get_glyph('P')
#define FONT_GLYPH_Q  font_get_glyph('Q')
#define FONT_GLYPH_R  font_get_glyph('R')
#define FONT_GLYPH_S  font_get_glyph('S')
#define FONT_GLYPH_T  font_get_glyph('T')
#define FONT_GLYPH_U  font_get_glyph('U')
#define FONT_GLYPH_V  font_get_glyph('V')
#define FONT_GLYPH_W  font_get_glyph('W')
#define FONT_GLYPH_X  font_get_glyph('X')
#define FONT_GLYPH_Y  font_get_glyph('Y')
#define FONT_GLYPH_Z  font_get_glyph('Z')

#define FONT_GLYPH_0  font_get_glyph('0')
#define FONT_GLYPH_1  font_get_glyph('1')
#define FONT_GLYPH_2  font_get_glyph('2')
#define FONT_GLYPH_3  font_get_glyph('3')
#define FONT_GLYPH_4  font_get_glyph('4')
#define FONT_GLYPH_5  font_get_glyph('5')
#define FONT_GLYPH_6  font_get_glyph('6')
#define FONT_GLYPH_7  font_get_glyph('7')
#define FONT_GLYPH_8  font_get_glyph('8')
#define FONT_GLYPH_9  font_get_glyph('9')

#define FONT_GLYPH_SPACE   font_get_glyph(' ')
#define FONT_GLYPH_COLON   font_get_glyph(':')
#define FONT_GLYPH_HYPHEN  font_get_glyph('-')
#define FONT_GLYPH_PERIOD  font_get_glyph('.')
#define FONT_GLYPH_SLASH   font_get_glyph('/')

#endif /* FONT_H_GUARD */
