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


/********************************************************************
 * Include files
 ********************************************************************/
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <syslog.h>
#include <termios.h>
#include <string.h>

#include "serial.h"
#include "ptc.h"
#include "dr7chk.h"
#include "ptcchk.h"
#include "update.h"


/********************************************************************
 *
 ********************************************************************/
int update (int ser, struct modemtype modem, char *UpdateFileName)
{
	char buffer[2 * CHUNKSIZE];

#ifdef DEBUG
	int r;
#endif

	char ch;
	uint16_t flashID;
	unsigned short chunks;
	unsigned long chunksWritten = 0;
	unsigned long bytesRead;

	int hFile;
	unsigned long fileLength;

	FDTIME fileStamp;
	FDTIME flashStamp;

	char *fext;

	hFile = open (UpdateFileName, O_RDONLY);

	if (-1 == hFile)
	{
		fprintf (stderr, "ERROR: opening file: %s\n", UpdateFileName);
		return -1;
	}

	// get the file extension
	fext = strrchr (UpdateFileName, '.');
	if (NULL == fext)
	{
		syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: Update file has no extension");
		fprintf (stderr, "ERROR: Update file has no extension.\n");
		close (hFile);
		return -1;
	}

#ifdef DEBUG
	printf ("Extension: %s\n", fext);
#endif

	// check if file extension matches the modem type
	if (!strncasecmp (fext, modem.ext, 3))
	{
		syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: file extension does not match modem type");
		fprintf (stderr, "ERROR: file extension does not match modem type.\n");
		close (hFile);
		return -1;
	}

	// check firmware file
	if (modem.ver == 'H' ||
		modem.ver == 'I' ||
		modem.ver == 'K')
	{
		if (dr7check (hFile))
		{
			syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: firmware CRC check failed");
			fprintf (stderr, "ERROR: firmware CRC check failed.\n");
			close (hFile);
			return -1;
		}
	}
	else
	{
		if (ptccheck (hFile))
		{
			syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: firmware CRC check failed");
			fprintf (stderr, "ERROR: firmware CRC check failed.\n");
			close (hFile);
			return -1;
		}
	}

	lseek (hFile, 12, SEEK_SET);
	read (hFile, &fileStamp, 4);

	fileLength = lseek (hFile, 0, SEEK_END);

	// start update on the modem
	write (ser, "UPDATE\r", 7);
	usleep (100000);

#ifdef DEBUG
	r = ser_flush (ser);	// read and ignore the UPDATE message
	printf ("flushed %d bytes\n", r);
#else
	ser_flush (ser);	// read and ignore the UPDATE message
#endif

	write (ser, "\006", 1);	// send ACK
	usleep (1000);

	read (ser, &flashID, 2);

#ifdef DEBUG
	printf ("flashID: %04X\n", flashID);
#endif

	read (ser, &flashStamp, 4);

#ifdef DEBUG
	printf ("File stamp : %08X\n", fileStamp);
	printf ("Flash stamp: %08X\n", flashStamp);
#endif

	// check for Flash ID 0xa41f
	// and for compatibility: 0x5b1f and 0xda1f
	if ((0xa41f != flashID) && (0x5b1f != flashID) && (0xda1f != flashID))
	{
		fprintf (stderr, "ERROR: receiving FlashID!\n");
		syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: receiving FlashID. Got %04X", flashID);
		write (ser, "\033", 1);	// send ESC
		close (hFile);
		return -1;
	}

	chunks = fileLength / CHUNKSIZE;

	if (fileLength % CHUNKSIZE)
	{
		chunks++;
	}

	if (flashStamp.day == 0 || flashStamp.month == 0 || (flashStamp.day == 0x1f && flashStamp.month == 0xf && flashStamp.year == 0x7f))
	{
		fprintf (stderr, "WARNING: Invalid Flash time stamp.\n"
						 "         Possibly no firmware installed.\n\n");
		syslog (LOG_MAKEPRI (LOG_USER, LOG_INFO), "WARNING: Invalid Flash time stamp. Possibly no firmware installed.");
	}

#ifdef CHECK_TIMESTAMP
	if (convtime (FileStamp) <= convtime (FlashStamp))
	{
		printf("The current firmware has the same or a newer time stamp!\n");
		printf("Press <P> to proceed or any other key to quit.\n\n");

		do
		{
			res = fgetc(stdin);
		}
		while (res == '\n' || res == '\r');

		if ('p' != (char)res && 'P' != (char)res)
		{
			write(ser, "\033", 1);	/* send ESC */
			close(hFile);
			return -2;
		}
	}
#endif /* CHECK_TIMESTAMP */

#ifdef CHECK_FILE_LENGTH
	if (fileLength > flashFree)
	{
		fprintf (stderr, "ERROR: File too large!\n       File should not be longer than %ld bytes.\n", flashFree);
		write (ser, "\033", 1);	// send ESC
		close (hFile);
		return -1;
	}
#endif /* CHECK_FILE_LENGTH */

#if 0
	// TEST: cancel update here
	write (ser, "\033", 1);	// send ESC
	close (hFile);

	fprintf (stderr, "TEST: Update canceled!\n");

	return 0;
#endif

	write (ser, "\006", 1);	// send ACK

	// write the number of chunks
	ch = (char) (chunks >> 8);
	write (ser, &ch, 1);
	ch = (char) chunks;
	write (ser, &ch, 1);

	read (ser, &ch, 1);

	if (ch != ACK)
	{
		fprintf (stderr, "\a\aERROR: Handshake failed!\n");
		syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: handshake failed. Rx: %02X", ch);
		write (ser, "\033", 1);	// send ESC
		close (hFile);
		return -1;
	}

	syslog (LOG_MAKEPRI (LOG_USER, LOG_INFO), "Updating with file: %s", UpdateFileName);

	printf ("Writing %ld byte in %d chunks.\n\n", fileLength, chunks);
	syslog (LOG_MAKEPRI (LOG_USER, LOG_INFO), "Writing %ld byte in %d chunks", fileLength, chunks);

	lseek (hFile, 0, SEEK_SET);

	do
	{
		bytesRead = read (hFile, &buffer, CHUNKSIZE);

		if (bytesRead < CHUNKSIZE)
		{
			memset (buffer + bytesRead, 0, CHUNKSIZE - bytesRead);
		}

		write (ser, &buffer, CHUNKSIZE);
		chunksWritten++;

		read (ser, &ch, 1);

		if (ch != ACK)
		{
			fprintf (stderr, "\a\aERROR: Handshake failed!\n");
			fprintf (stderr, "Char: %02X\n", ch);
			write (ser, "\033", 1);	// send ESC
			close (hFile);
			return -1;
		}

		sprintf (buffer, "Written: %3ld%%\r", chunksWritten * 100 / chunks);
		write (STDOUT_FILENO, buffer, strlen (buffer));

	} while (bytesRead == CHUNKSIZE);

	write (ser, "\r", 1);

	printf ("\n\n\aUpdate complete.\n");

	close (hFile);

	return 0;
}

#ifdef CHECK_TIMESTAMP
/********************************************************************
 *
 ********************************************************************/
time_t convtime (FDTIME PTC_Time)
{
	struct tm btime;

	btime.tm_sec = PTC_Time.twosecs * 2;
	btime.tm_min = PTC_Time.minutes;
	btime.tm_hour = PTC_Time.hours;
	btime.tm_mday = PTC_Time.day;
	btime.tm_mon = PTC_Time.month;
	btime.tm_year = PTC_Time.year + 80;
	btime.tm_wday = 0;
	btime.tm_yday = 0;
	btime.tm_isdst = 0;

#ifdef DEBUG
	fputs (asctime (&btime), stdout);
#endif

	return mktime (&btime);
}
#endif
