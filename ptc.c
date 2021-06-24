/********************************************************************
 *
 * ptc.c -- PTC utilities for command (cmd:) mode
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

#define _GNU_SOURCE

/********************************************************************
 * Include files
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "serial.h"
#include "ptc.h"


/********************************************************************
 * Global variables
 ********************************************************************/
static struct modemtype modems[] = {
	//Type	Name           Ext.  HM-Log
	{'A',	"PTC-II",     "pt2", false},
	{'B',	"PTC-IIpro",  "pro", false},
	{'C',	"PTC-IIe",    "pte", false},
	{'D',	"PTC-IIex",   "pex", false},
	{'E',	"PTC-IIusb",  "ptu", false},
	{'F',	"PTC-IInet",  "ptn", false},
	{'H',	"DR-7800",    "dr7", true},
	{'I',	"DR-7400",    "dr7", true},
	{'K',	"DR-7000",    "dr7", true},
	{'L',	"PTC-IIIusb", "p3u", false},
	{'T',	"PTC-IItrx",  "ptx", false},
};

//struct modemtype modem = {
//	0, NULL, NULL, false
//};


/********************************************************************
 * Send a command to the PTC (short for PACTOR Controller)
 * and wait for the given string
 *
 * Return 0 = Ok
 *       -1 = Error
 ********************************************************************/
int PTC_cmd (int ser, char *cmd, size_t len)
{
	int res;

	write (ser, cmd, len);
	res = ser_wait (ser, CMDSTR);

	if (res)
	{
		syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: timeout waiting for >%s<", cmd);
	}

	return res;
}


/********************************************************************
 * Feed the modem with commands from a file
 ********************************************************************/
void PTC_file (int ser, char *filename)
{
	FILE *f;
	char *line = NULL;
	size_t len = 0;

	// TODO: was mache ich hier mit den Kommandos die beim Start des HM automatisch gesetzt werden?

	f = fopen (filename, "r");

	if (f == NULL)
	{
		syslog (LOG_MAKEPRI (LOG_USER, LOG_INFO), "INFO: could not open file >%s<", filename);
		return;
	}

	while (getline (&line, &len, f) != -1)
	{
		line[len - 1] = '\r';			// /n -> /r

		PTC_cmd (ser, line, len);
	}

	fclose (f);
}


/********************************************************************
 * Set date and time of modem
 ********************************************************************/
void PTC_setTime (int ser, bool UTC)
{
	#define TBUFMAX 40
	time_t ct;
	struct tm *ptm;
	char buf[TBUFMAX];
	int n;

	ct = time (NULL);

	if (UTC)
	{
		ptm = gmtime (&ct);
	}
	else
	{
		ptm = localtime (&ct);
	}

	n = snprintf (buf, TBUFMAX, "date %02d%02d%02d\r", ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year - 100);
	if (n > 12)
	{
		syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: in date string (length %d)", n);
	}
	PTC_cmd (ser, buf, 12);

	n = snprintf (buf, TBUFMAX, "time %02d%02d%02d\r", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	if (n > 12)
	{
		syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: in time string (length %d)", n);
	}
	PTC_cmd (ser, buf, 12);

	syslog (LOG_MAKEPRI (LOG_USER, LOG_INFO), "PTC date & time set to: %02d.%02d.%02d %02d:%02d:%02d", ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year - 100, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}


/********************************************************************
 * Get the version string of the modem
 ********************************************************************/
struct modemtype PTC_getVersion (int ser)
{
	#define BUFMAX 256
	char buf[BUFMAX];
	int len;
	int i;
	char modemType = 0;
	struct modemtype modem = {
		0, NULL, NULL, false
	};

	write (ser, "ver ##\r", 7);
	while ((len = ser_getwait (ser, CMDSTR, buf)))
	{
		if (len > 2 && buf[0] == '#' && buf[1] == '0' && buf[2] == ':')
		{
			modemType = buf[3];
		}
	}

	for (i = 0; i < (sizeof(modems) / sizeof(struct modemtype)); i++)
	{
		if (modemType == modems[i].ver)
		{
			modem = modems[i];
			break;
		}
	}

	if (modem.ver)
	{
		syslog (LOG_MAKEPRI (LOG_USER, LOG_INFO), "Modem detected: %s", modem.name);
	}
	else
	{
		syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: unknown modem type: %c", modemType);
	}

	return modem;
}


/********************************************************************
 * Get the Hostmode PACTOR channel
 * Return
 *   1... 31 - PACTOR channel
 *   negative = Error
 ********************************************************************/
int PTC_getPTChn (int ser)
{
	int len;
	char buf[40];
	int ptc = -1;
	char *p;

	write (ser, "ptc\r", 4);
	while ((len = ser_getwait (ser, CMDSTR, buf)))
	{
		if (len > 2 && buf[0] == '*' && buf[1] == '*' && buf[2] == '*')
		{
			buf[len - 2] = 0;
			p = strrchr (buf, ' ');
			ptc = atoi (++p);
		}
	}

	return ptc;
}


/********************************************************************
 * Get the modem serial number
 * Return:
 *   true  - Serial number ok
 *   false - Error = serial number not valid
 ********************************************************************/
bool PTC_getSerNum (int ser, uint64_t *sernum)
{
	int len;
	char buf[40];
	char *p;
	bool ret = false;

	write (ser, "sys sern\r", 9);
	while ((len = ser_getwait (ser, CMDSTR, buf)))
	{
		if (len > 2 && buf[0] == 'S' && buf[1] == 'e' && buf[2] == 'r')
		{
			buf[len - 2] = 0;
			p = strrchr (buf, ' ');
			*sernum = strtoull (++p, NULL, 16);
			ret = true;
		}
	}

	return ret;
}
