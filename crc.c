/********************************************************************
 *
 * crc.c -- CRC function
 *
 * Copyright (C) 2013 SCS GmbH & Co. KG, Hanau, Germany
 * written by Peter Mack (peter.mack@scs-ptc.com)
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ********************************************************************/


/********************************************************************
 * Include files
 ********************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "crc.h"


/********************************************************************
 * Global Variables
 ********************************************************************/
uint32_t crctable[UCHAR_MAX + 1];
uint32_t crc;


/********************************************************************
 *
 ********************************************************************/
void make_crctable (void)
{
	unsigned int i, j;
	uint32_t r;

	for (i = 0; i <= UCHAR_MAX; i++)
	{
		r = i;
		for (j = CHAR_BIT; j > 0; j--)
		{
			if (r & 1)
			{
				r = (r >> 1) ^ CRCPOLY;
			}
			else
			{
				r >>= 1;
			}
		}
		crctable[i] = r;
	}
}

/********************************************************************
 *
 ********************************************************************/
uint8_t get_byte (int f)
{
	uint8_t b;

	if (read (f, &b, 1) < 0)
	{
		fprintf (stderr, "ERROR: reading file\n");
	}
	return b;
}

/********************************************************************
 *
 ********************************************************************/
uint16_t get_word (int f)
{
	uint16_t b;

	if (read (f, &b, 2) < 0)
	{
		fprintf (stderr, "ERROR: reading file\n");
	}
	return b;
}

/********************************************************************
 *
 ********************************************************************/
uint32_t get_long (int f)
{
	uint32_t b;

	if (read (f, &b, 4) < 0)
	{
		fprintf (stderr, "ERROR: reading file\n");
	}
	return b;
}
