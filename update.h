/********************************************************************
 *
 * update.c -- SCS modem update handling
 *
 * Copyright (C) 1998 - 2021 SCS GmbH & Co. KG, Hanau, Germany
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

#pragma once

/********************************************************************
 * Include files
 ********************************************************************/
#include "ptc.h"


/********************************************************************
 * Defines
 ********************************************************************/
#define CHUNKSIZE 256

#define ACK '\006'
#define ESC '\033'


/********************************************************************
 * Types
 ********************************************************************/
typedef struct _FDTIME
{
	unsigned int twosecs :5;
	unsigned int minutes :6;
	unsigned int hours :5;
	unsigned int day :5;
	unsigned int month :4;
	unsigned int year :7;	// = year - 1980
} FDTIME;


/********************************************************************
 * Function Prototypes
 ********************************************************************/
int update (int ser, struct modemtype modem, char *UpdateFileName);
#ifdef CHECK_TIMESTAMP
time_t convtime (FDTIME PTC_Time);
#endif /* CHECK_TIMESTAMP */
