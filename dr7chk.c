/********************************************************************
 *
 * dr7chk.c -- SCS P4dragon firmware check
 *
 * Copyright (C) 2013 -2021 SCS GmbH & Co. KG, Hanau, Germany
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
#include "dr7chk.h"


/********************************************************************
 * Defines
 ********************************************************************/
#define HEADER_P4	0x3450


/********************************************************************
 * Global variables
 ********************************************************************/
extern uint32_t crctable[UCHAR_MAX + 1];
extern uint32_t crc;


/********************************************************************
 * Quick header check
 * Return:
 *  0 = Ok
 *  negative = Error
 ********************************************************************/
int dr7check (int f)
{
	long unsigned int size;

	make_crctable ();

	if (HEADER_P4 != get_word (f))
	{
		close (f);
		fprintf (stderr, "ERROR: Wrong header ID.\n");	// ERROR: file have to start with the P4 header
		return -1;
	}

	get_word (f);	// dummy read: number of parts

	// calculate CRC
	size = get_long (f);
	lseek (f, 0, SEEK_SET);
	crc = CRC_MASK;
	while (size--)
	{
		UPDATE_CRC(crc, get_byte (f));
	}

	if ((crc ^ CRC_MASK) != get_long (f))
	{
		close (f);
		fprintf (stderr, "ERROR: wrong CRC.\n");
		return -2;
	}

	return 0;
}
