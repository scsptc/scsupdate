/********************************************************************
 *
 * serial.h -- Serial port handling
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

#pragma once

/********************************************************************
 * Function prototypes
 ********************************************************************/
int ser_open (char *serdev, int baud);
void ser_close (int ser, char *serdev);

int ser_set_baud (int ser, int baud);
int ser_set_stopbits (int ser, int stop_bit);
int ser_set_parity (int ser, char parity);

void ser_set_dtr (int ser, int i);
void ser_set_rts (int ser, int i);

int ser_get_dcd (int ser);
int ser_get_ri (int ser);
int ser_get_dsr (int ser);
int ser_get_cts (int ser);

int ser_flush (int ser);
int ser_wait (int ser, const char *cmd);
int ser_getwait (int ser, const char *cmd, char *p);
