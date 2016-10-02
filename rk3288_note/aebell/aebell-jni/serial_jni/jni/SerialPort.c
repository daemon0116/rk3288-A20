
#include"jni.h"


#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "SerialPort.h"


static speed_t getBaudrate(jint baudrate)
{
	switch(baudrate) {
	case 0: return B0;
	case 50: return B50;
	case 75: return B75;
	case 110: return B110;
	case 134: return B134;
	case 150: return B150;
	case 200: return B200;
	case 300: return B300;
	case 600: return B600;
	case 1200: return B1200;
	case 1800: return B1800;
	case 2400: return B2400;
	case 4800: return B4800;
	case 9600: return B9600;
	case 19200: return B19200;
	case 38400: return B38400;
	case 57600: return B57600;
	case 115200: return B115200;
	case 230400: return B230400;
	case 460800: return B460800;
	case 500000: return B500000;
	case 576000: return B576000;
	case 921600: return B921600;
	case 1000000: return B1000000;
	case 1152000: return B1152000;
	case 1500000: return B1500000;
	case 2000000: return B2000000;
	case 2500000: return B2500000;
	case 3000000: return B3000000;
	case 3500000: return B3500000;
	case 4000000: return B4000000;
	default: return -1;
	}
}


int SerialOpen(char *filename, int BaudRate, enDataBits eDataBits, enStopBits eStopBits, enParityCheck eParityCheck)
{

	speed_t speed;

	/* Check arguments */
	{
		speed = getBaudrate(BaudRate);
		if (speed == -1) {
			return 0;
		}
	}

    
    int fd;
    struct termios Opt;
    int s32Status;
    fd = open(filename, O_RDWR); //O_RDWR   | O_NOCTTY | O_NDELAY
    if (fd <= 0)
    {
        return 0;
    }
    tcgetattr(fd, &Opt);
    tcflush(fd, TCIOFLUSH);
    Opt.c_cflag &= ~CSIZE;
    Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //Input
    Opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    Opt.c_oflag &= ~OPOST; //Output
    tcsetattr(fd, TCSANOW, &Opt);
    tcgetattr(fd, &Opt);
    tcflush(fd, TCIOFLUSH);
    cfsetispeed(&Opt, speed);
    cfsetospeed(&Opt, speed);
    s32Status = tcsetattr(fd, TCSANOW, &Opt);
    if (s32Status != 0)
    {
        close(fd);
        return 0;
    }
    s32Status = tcgetattr(fd, &Opt);
    Opt.c_cflag &= ~CSIZE;
    /*set the databits*/
    switch (eDataBits)
    {
    case DBIT_5:
    case DBIT_6:
    case DBIT_7:
        Opt.c_cflag |= CS7;
        break;
    case DBIT_8:
        Opt.c_cflag |= CS8;
        break;
    default:
        close(fd);
        return 0;
    }
    /*set the parity*/
    switch (eParityCheck)
    {
    case PCHK_MARK:
    case PCHK_SPACE:
        Opt.c_cflag &= ~PARENB; /* Clear parity enable */
        Opt.c_iflag &= ~INPCK; /* Enable parity checking */
        break;
    case PCHK_ODD:
        Opt.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
        Opt.c_iflag |= INPCK; /* Disnable parity checking */
        break;
    case PCHK_EVEN:
        Opt.c_cflag |= PARENB; /* Enable parity */
        Opt.c_cflag &= ~PARODD; /* 转换为偶效验*/
        Opt.c_iflag |= INPCK; /* Disnable parity checking */
        break;
    case PCHK_NONE: /*as no parity*/
        Opt.c_cflag &= ~PARENB;
        Opt.c_cflag &= ~CSTOPB;
        break;
    default:
        close(fd);
        return 0;
    }
    /*set the stopbits*/
    switch (eStopBits)
    {
    case SBIT_1:
        Opt.c_cflag &= ~CSTOPB;
        break;
    case SBIT_1P5:
    case SBIT_2:
        Opt.c_cflag |= CSTOPB;
        break;
    default:
        close(fd);
        return 0;
    }
    /* Set input parity option */
    if (eParityCheck != PCHK_MARK && eParityCheck != PCHK_SPACE)
    {
        Opt.c_iflag |= INPCK;
    }
    Opt.c_cc[VTIME] = 1; // 100ms
    Opt.c_cc[VMIN] = 8;
    tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
    s32Status = tcsetattr(fd, TCSANOW, &Opt);
    if (s32Status != 0)
    {
        close(fd);
        return 0;
    }

    return fd;
}


