#ifndef _SERIAL_PORT_H_
#define _SERIAL_PORT_H_

//������
typedef enum
{
	BR_300,
	BR_600,
	BR_1200,
	BR_1800,
	BR_2400,
	BR_4800,
	BR_9600,
	BR_19200,
	BR_38400,
	BR_57600,
	BR_115200,
	BR_MAX
}enBaudRate;

//����λ
typedef enum
{
	DBIT_5,
	DBIT_6,
	DBIT_7,
	DBIT_8
}enDataBits;

//ֹͣλ
typedef enum
{
	SBIT_1,
	SBIT_1P5,
	SBIT_2
}enStopBits;

//У��λ
typedef enum
{
	PCHK_NONE,
	PCHK_ODD,
	PCHK_EVEN,
	PCHK_MARK,
	PCHK_SPACE
}enParityCheck;

int SerialOpen(char *filename, int BaudRate, enDataBits eDataBits, enStopBits eStopBits, enParityCheck eParityCheck);

#endif
