/*
    i2cset.c - A user-space program to write an I2C register.
    Copyright (C) 2001-2003  Frodo Looijaard <frodol@dds.nl>, and
                             Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2004-2005  Jean Delvare <khali@linux-fr.org>

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

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "i2cbusses.h"
#include "util.h"
#include <linux/i2c-dev.h>
#include "../version.h"

static void help(void) __attribute__ ((noreturn));

static void help(void)
{
	fprintf(stderr, "Syntax: i2cset [-f] [-y] I2CBUS CHIP-ADDRESS DATA-ADDRESS "
	        "VALUE [MODE] [MASK]\n"
	        "        i2cset -V\n"
	        "  MODE is 'b[yte]' or 'w[ord]' (default b)\n"
	        "  Append 'p' to MODE for PEC checking\n"
	        "  I2CBUS is an integer\n");
	print_i2c_busses(0);
	exit(1);
}

int main(int argc, char *argv[])
{
	char *end;
	int res, i2cbus, address, size, file;
	int value, daddress, vmask = 0;
	char filename[20];
	unsigned long funcs;
	int pec = 0;
	int flags = 0;
	int force = 0, yes = 0, version = 0;

	/* handle (optional) flags first */
	while (1+flags < argc && argv[1+flags][0] == '-') {
		switch (argv[1+flags][1]) {
		case 'V': version = 1; break;
		case 'f': force = 1; break;
		case 'y': yes = 1; break;
		default:
			fprintf(stderr, "Warning: Unsupported flag "
				"\"-%c\"!\n", argv[1+flags][1]);
			help();
			exit(1);
		}
		flags++;
	}

	if (version) {
		fprintf(stderr, "i2cset version %s\n", VERSION);
		exit(0);
	}

	if (argc < flags + 5)
		help();

	i2cbus = strtol(argv[flags+1], &end, 0);
	if (*end || i2cbus < 0 || i2cbus > 0x3f) {
		fprintf(stderr, "Error: I2CBUS argument invalid!\n");
		help();
	}

	address = strtol(argv[flags+2], &end, 0);
	if (*end || address < 0 || address > 0x7f) {
		fprintf(stderr, "Error: Chip address invalid!\n");
		help();
	}

	daddress = strtol(argv[flags+3], &end, 0);
	if (*end || daddress < 0 || daddress > 0xff) {
		fprintf(stderr, "Error: Data address invalid!\n");
		help();
	}

	value = strtol(argv[flags+4], &end, 0);
	if (*end) {
		fprintf(stderr, "Error: Data value invalid!\n");
		help();
	}

	if (argc < flags + 6) {
		fprintf(stderr, "No size specified (using byte-data access)\n");
		size = I2C_SMBUS_BYTE_DATA;
	} else if (argv[flags+5][0] == 'b') {
		size = I2C_SMBUS_BYTE_DATA;
		pec = argv[flags+5][1] == 'p';
	} else if (argv[flags+5][0] == 'w') {
		size = I2C_SMBUS_WORD_DATA;
		pec = argv[flags+5][1] == 'p';
	} else {
		fprintf(stderr, "Error: Invalid mode!\n");
		help();
	}

	if (argc >= flags + 7) {
		vmask = strtol(argv[flags+6], &end, 0);
		if (*end || vmask == 0) {
			fprintf(stderr, "Error: Data value mask invalid!\n");
			help();
		}
	}

	if (value < 0
	 || (size == I2C_SMBUS_BYTE_DATA && value > 0xff)
	 || (size == I2C_SMBUS_WORD_DATA && value > 0xffff)) {
		fprintf(stderr, "Error: Data value out of range!\n");
		help();
	}

	file = open_i2c_dev(i2cbus, filename, 0);
	if (file < 0) {
		exit(1);
	}

	/* check adapter functionality */
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
		        "functionality matrix: %s\n", strerror(errno));
		exit(1);
	}

	switch (size) {
	case I2C_SMBUS_BYTE_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE_DATA)) {
			fprintf(stderr, "Error: Adapter for i2c bus %d does "
			        "not have byte write capability\n", i2cbus);
			exit(1);
		}
		break;

	case I2C_SMBUS_WORD_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_WORD_DATA)) {
			fprintf(stderr, "Error: Adapter for i2c bus %d does "
			        "not have word write capability\n", i2cbus);
			exit(1);
		}
		break;
	}

	if (set_slave_addr(file, address, force) < 0)
		exit(1);

	if (!yes) {
		int dont = 0;

		fprintf(stderr, "WARNING! This program can confuse your I2C "
		        "bus, cause data loss and worse!\n");

		if (address >= 0x50 && address <= 0x57) {
			fprintf(stderr, "DANGEROUS! Writing to a serial "
			        "EEPROM on a memory DIMM\nmay render your "
			        "memory USELESS and make your system "
			        "UNBOOTABLE!\n");
			dont = 1;
		}

		fprintf(stderr, "I will write to device file %s, chip address "
		        "0x%02x, data address\n0x%02x, data 0x%02x%s, mode "
		        "%s.\n", filename, address, daddress, value,
			vmask ? " (masked)" : "",
			size == I2C_SMBUS_BYTE_DATA ? "byte" : "word");
		if (pec)
			fprintf(stderr, "PEC checking enabled.\n");

		fprintf(stderr, "Continue? [%s] ", dont ? "y/N" : "Y/n");
		fflush(stderr);
		if (!user_ack(!dont)) {
			fprintf(stderr, "Aborting on user request.\n");
			exit(0);
		}
	}

	if (vmask) {
		int oldvalue;

		if (size == I2C_SMBUS_WORD_DATA) {
			oldvalue = i2c_smbus_read_word_data(file, daddress);
		} else {
			oldvalue = i2c_smbus_read_byte_data(file, daddress);
		}

		if (oldvalue < 0) {
			fprintf(stderr, "Error: Failed to read old value\n");
			exit(1);
		}

		value = (value & vmask) | (oldvalue & ~vmask);

		if (!yes) {
			fprintf(stderr, "Old value 0x%0*x, write mask "
				"0x%0*x: Will write 0x%0*x to register "
				"0x%02x\n",
				size == I2C_SMBUS_WORD_DATA ? 4 : 2, oldvalue,
				size == I2C_SMBUS_WORD_DATA ? 4 : 2, vmask,
				size == I2C_SMBUS_WORD_DATA ? 4 : 2, value,
				daddress);

			fprintf(stderr, "Continue? [Y/n] ");
			fflush(stderr);
			if (!user_ack(1)) {
				fprintf(stderr, "Aborting on user request.\n");
				exit(0);
			}
		}
	}

	if (pec) {
		if (ioctl(file, I2C_PEC, 1) < 0) {
			fprintf(stderr, "Error: Could not set PEC: %s\n",
			        strerror(errno));
			close(file);
			exit(1);
		}
		if (!(funcs & (I2C_FUNC_SMBUS_PEC | I2C_FUNC_I2C))) {
			fprintf(stderr, "Warning: Adapter for i2c bus %d does "
			        "not seem to actually support PEC\n", i2cbus);
		}
	}

	if (size == I2C_SMBUS_WORD_DATA) {
		res = i2c_smbus_write_word_data(file, daddress, value);
	} else {
		res = i2c_smbus_write_byte_data(file, daddress, value);
	}
	if (res < 0) {
		fprintf(stderr, "Error: Write failed\n");
		close(file);
		exit(1);
	}

	if (pec) {
		if (ioctl(file, I2C_PEC, 0) < 0) {
			fprintf(stderr, "Error: Could not clear PEC: %s\n",
				strerror(errno));
			close(file);
			exit(1);
		}
	}

	if (size == I2C_SMBUS_WORD_DATA) {
		res = i2c_smbus_read_word_data(file, daddress);
	} else {
		res = i2c_smbus_read_byte_data(file, daddress);
	}
	close(file);

	if (res < 0) {
		printf("Warning - readback failed\n");
	} else
	if (res != value) {
		printf("Warning - data mismatch - wrote "
		       "0x%0*x, read back 0x%0*x\n",
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, value,
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, res);
	} else {
		printf("Value 0x%0*x written, readback matched\n",
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, value);
	}

	exit(0);
}
