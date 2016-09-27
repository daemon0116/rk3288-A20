/*
    i2cbusses.h - Part of the i2c-tools package

    Copyright (C) 2004-2007  Jean Delvare <khali@linux-fr.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#ifndef _I2CBUSSES_H
#define _I2CBUSSES_H

void print_i2c_busses(int procfmt);

int open_i2c_dev(const int i2cbus, char *filename, const int quiet);
int set_slave_addr(int file, int address, int force);

#endif
