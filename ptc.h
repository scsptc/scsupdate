/********************************************************************
 *
 * ptc.c -- PTC utilities for cmd: mode
 *
 * Copyright (C) 2020-2021 SCS GmbH & Co. KG, Hanau, Germany
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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


/********************************************************************
 * Defines
 ********************************************************************/
#define CMDSTR	"cmd: "


/********************************************************************
 * Types
 ********************************************************************/
struct modemtype {
	char ver;
	char *name;
	char *ext;
	bool log;
};


/********************************************************************
 * Function prototypes
 ********************************************************************/
int PTC_cmd (int ser, char *cmd, size_t len);
void PTC_file (int ser, char *filename);
void PTC_setTime (int ser, bool UTC);
struct modemtype PTC_getVersion (int ser);
int PTC_getPTChn (int ser);
bool PTC_getSerNum (int ser, uint64_t *sernum);
