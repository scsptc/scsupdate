/********************************************************************
 *
 * serial.c -- Serial port handling
 *
 * Copyright (C) 2015-2021 SCS GmbH & Co. KG, Hanau, Germany
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
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm-generic/termbits.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef __linux__
#include "lock.h"	// handle UUCP style lock files
#endif /* __linux__ */
#include "serial.h"


/********************************************************************
 * ser_open
 *  open a serial device and configure it
 *
 *  Return 0 = Ok
 *        -1 = Error
 ********************************************************************/
int ser_open (char *serdev, int baud)
{
	int ser;
	struct termios2 options;
	int r;

	//printf ("Open serial device %s\n", serdev);
	syslog (LOG_MAKEPRI(LOG_USER, LOG_INFO), "Open serial device %s", serdev);
	// serial device
#ifdef __linux__
	if (lock_device (serdev) < 0)
	{
		// Error
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: device %s is locked", serdev);
		return -1;
	}
#endif /* __linux__ */

	if ((ser = open (serdev, O_RDWR | O_NOCTTY)) < 0)
	{
		// Error
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: could not open %s: %s", serdev, strerror (errno));

#ifdef __linux__
		unlock_device (serdev);
#endif /* __linux__ */
		return -1;
	}

	r = ioctl (ser, TCGETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCGETS2 - %s", strerror (errno));
		close (ser);
		return -1;
	}

	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1;	// block and read 1 char
	options.c_iflag = 0;
	options.c_iflag |= IGNBRK;
	options.c_oflag = 0;
	options.c_lflag = 0;
	options.c_cflag &= ~(
			CSIZE |
			CSTOPB |	// 1 stop bit
			CRTSCTS |	// disable RTS/CTS flow control
			PARENB |	// disable parity
			PARODD |	// (even parity)
			HUPCL		// lower modem control lines after last process closes the device (hang up)
			);
	options.c_cflag |= (
			CS8 |		// 8 data bits
			CREAD |		// enable receiver
			CLOCAL		// ignore modem control lines (CD)
			);

	options.c_cflag &= ~CBAUD;
	options.c_cflag |= CBAUDEX;

	options.c_ispeed = baud;
	options.c_ospeed = baud;

	r = ioctl (ser, TCSETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCSETS2 - %s", strerror (errno));
		close (ser);
		return -1;
	}

	syslog (LOG_MAKEPRI(LOG_USER, LOG_INFO), "serial device %s opened", serdev);

	return ser;
}


/********************************************************************
 * ser_close
 *  close a serial device
 ********************************************************************/
void ser_close (int ser, char *serdev)
{
	close (ser);

#ifdef __linux__
	unlock_device (serdev);
#endif /* __linux__ */

	syslog (LOG_MAKEPRI(LOG_USER, LOG_INFO), "serial device %s closed", serdev);
}


/********************************************************************
 * ser_set_baud
 *  set baudrate on a serial device
 *
 *  Return 0 = Ok
 *        -1 = Error
 ********************************************************************/
int ser_set_baud (int ser, int baud)
{
	int r;
	struct termios2 options;

	r = ioctl (ser, TCGETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCGETS2 - %s", strerror (errno));
		return -1;
	}

	options.c_cflag &= ~CBAUD;
	options.c_cflag |= CBAUDEX;

	options.c_ispeed = baud;
	options.c_ospeed = baud;

	r = ioctl (ser, TCSETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCSETS2 - %s", strerror (errno));
		return -1;
	}

	return 0;
}


/********************************************************************
 *
 ********************************************************************/
int ser_set_stopbits (int ser, int stop_bits)
{
	struct termios2 options;
	int r;

	r = ioctl (ser, TCGETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCGETS2 - %s", strerror (errno));
		return -1;
	}

	switch (stop_bits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;	// 1 stop bit
			break;

		case 2:
			options.c_cflag |= CSTOPB;	// 2 stop bits
			break;

		default:
			syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: unsupported stop bits - %d", stop_bits);
			return -1;
	}

	r = ioctl (ser, TCSETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCSETS2 - %s", strerror (errno));
		return -1;
	}

	return 0;
}


/********************************************************************
 *
 ********************************************************************/
int ser_set_parity (int ser, char parity)
{
	struct termios2 options;
	int r;

	r = ioctl (ser, TCGETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCGETS2 - %s", strerror (errno));
		return -1;
	}

	switch (parity)
	{
		case 'N':
		case 'n':
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;

		case 'E':
		case 'e':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;

		case 'O':
		case 'o':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;

		default:
			return -1;
	}

	r = ioctl (ser, TCSETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCSETS2 - %s", strerror (errno));
		return -1;
	}

	return 0;
}


/********************************************************************
 *
 ********************************************************************/
void ser_set_dtr (int ser, int i)
{
	int bit = TIOCM_DTR;

	if (i)
	{
		ioctl (ser, TIOCMBIS, &bit);	// set DTR
	}
	else
	{
		ioctl (ser, TIOCMBIC, &bit);	// clear DTR
	}
}


/********************************************************************
 *
 ********************************************************************/
void ser_set_rts (int ser, int i)
{
	int bit = TIOCM_RTS;

	if (i)
	{
		ioctl (ser, TIOCMBIS, &bit);	// set RTS
	}
	else
	{
		ioctl (ser, TIOCMBIC, &bit);	// clear RTS
	}
}


/********************************************************************
 *
 ********************************************************************/
int ser_get_dcd (int ser)
{
	int status;

	ioctl (ser, TIOCMGET, &status);

	return (status & TIOCM_CAR);
}


/********************************************************************
 *
 ********************************************************************/
int ser_get_ri (int ser)
{
	int status;

	ioctl (ser, TIOCMGET, &status);

	return (status & TIOCM_RNG);
}


/********************************************************************
 *
 ********************************************************************/
int ser_get_dsr (int ser)
{
	int status;

	ioctl (ser, TIOCMGET, &status);

	return (status & TIOCM_DSR);
}


/********************************************************************
 *
 ********************************************************************/
int ser_get_cts (int ser)
{
	int status;

	ioctl (ser, TIOCMGET, &status);

	return (status & TIOCM_CTS);
}


/********************************************************************
 * Flush serial port
 * Return:
 *  number of flushed bytes
 *  negative = Error
 ********************************************************************/
int ser_flush (int ser)
{
	int flushed = 0;
	struct termios2 options;
	int r;
	char c;

	r = ioctl (ser, TCGETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCGETS2 - %s", strerror (errno));
		close (ser);
		return -1;
	}

	options.c_cc[VTIME] = 2;	// 200 ms timeout
	options.c_cc[VMIN] = 0;

	r = ioctl (ser, TCSETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCSETS2 - %s", strerror (errno));
		close (ser);
		return -1;
	}

	do
	{
		r = read (ser, &c, 1);
		flushed += r;
	}
	while (r > 0);

	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1;	// block and read 1 char

	r = ioctl (ser, TCSETS2, &options);
	if (r < 0)
	{
		syslog (LOG_MAKEPRI(LOG_USER, LOG_ERR), "ERROR: TCSETS2 - %s", strerror (errno));
		close (ser);
		return -1;
	}

	return flushed;
}


/********************************************************************
 * Wait for a given string
 *
 * Return 0 = Ok
 *       -1 = Error
 ********************************************************************/
int ser_wait (int ser, const char *cmd)
{
	ssize_t r;
	char c;
	int x;
	int l;
	int run;

	l = strlen (cmd);
	run = 1;
	x = 0;
	while (run)
	{
		r = read (ser, &c, 1);
		if (0 == r)
		{
			syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: timeout occured. Waiting for: %s", cmd);
			return -1;
		}
		if (c == cmd[x])
		{
			x++;
			if (x == l)
			{
				run = 0;
			}
		}
		else
		{
			x = 0;
		}
	}
	return 0;
}


/********************************************************************
 * wait for a given string
 * and return every captured line
 ********************************************************************/
int ser_getwait (int ser, const char *cmd, char *p)
{
	ssize_t r;
	char c;
	int x;
	int l;
	int run;
	int i;

	l = strlen (cmd);
	run = 1;
	x = 0;
	i = 0;
	while (run)
	{
		r = read (ser, &c, 1);
		if (0 == r)
		{
			syslog (LOG_MAKEPRI (LOG_USER, LOG_ERR), "ERROR: timeout occured. Waiting for: %s", cmd);
			return -1;
		}
		*p++ = c;
		i++;
		if (c == cmd[x])
		{
			x++;
			if (x == l)
			{
				run = 0;
				i = 0;
			}
		}
		else
		{
			x = 0;
			if (c == '\n')
			{
				run = 0;
				p -= 2;
				*p = '\0';
			}
		}
	}
	return i;
}
