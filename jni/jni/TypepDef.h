#ifndef __INC_TYPDEF#define __INC_TYPDEF//#define OK            0//#define ERROR       (-1)//#define	TRUE 	1//#define	FALSE 	0#undef NULL#if defined(__cplusplus)#define NULL 0#else							/*  */#define NULL ((void *)0)#endif							/*  */#define LenArray(arr)  (sizeof(arr)/sizeof(arr[0]))#define LenArrayRow(arr)  (sizeof(arr)/sizeof(arr[0]))	//计算二维数组的行长度#define LenArrayCol(arr)  (sizeof(arr[0])/(sizeof(arr[0][0])))	//计算二维数组的列长度
typedef unsigned char  BYTE;	// 8-bit unsigned
typedef unsigned short WORD;	// 16-bit unsigned
typedef unsigned int   DWORD;	// 32-bit unsigned
typedef unsigned long long QWORD;	// 64-bit unsignedtypedef char CHAR;		// 8-bit signedtypedef unsigned char UCHAR;		// 8-bit signedtypedef short int SHORT;	// 16-bit signedtypedef long LONG;		// 32-bit signedtypedef long long LONGLONG;	// 64-bit signedtypedef unsigned long	ULONG;typedef void* (*pFUNC)(void *p);
/* Alternate definitions */typedef void VOID;typedef char CHAR8;
typedef unsigned char UCHAR8;

/* Processor & Compiler independent, size specific definitions */// To Do:  We need to verify the sizes on each compiler.  These//         may be compiler specific, we should either move them//         to "compiler.h" or #ifdef them for compiler type.typedef int INT;
typedef char INT8;
typedef short INT16;
typedef int INT32;
typedef signed long long INT64;typedef const char CINT8;
typedef unsigned int UINT;
typedef unsigned char UINT8;
typedef unsigned short  UINT16;
typedef unsigned int UINT32;	// other name for 32-bit integer
typedef unsigned long UINT32L;typedef unsigned long long UINT64;
/* These types must be 16-bit integer */typedef unsigned short	WCHAR;typedef enum {FALSE = 0, TRUE = !FALSE, TRUE2} bool;//typedef char *                LPCSTR;
#ifndef BOOLtypedef bool BOOL;#endif							/* */typedef unsigned char  uint8;  /* defined for unsigned 8-bits integer variable 	无符号8位整型变量  */typedef          char  int8;   /* defined for signed 8-bits integer variable		有符号8位整型变量  */typedef unsigned short uint16; /* defined for unsigned 16-bits integer variable 	无符号16位整型变量 */typedef          short int16;  /* defined for signed 16-bits integer variable 		有符号16位整型变量 */typedef unsigned int   uint32; /* defined for unsigned 32-bits integer variable 	无符号32位整型变量 */typedef          int   int32;  /* defined for signed 32-bits integer variable 		有符号32位整型变量 */typedef float          fp32;   /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */typedef double         fp64;   /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */typedef unsigned short uint16_t;typedef unsigned int   uint32_t;/* *定义用于位操作的宏 */#define SET_BIT(data, bit)			((data) |= (0x01u << (bit)))#define CLR_BIT(data, bit)			((data) &= (~(0x01u << (bit))))#define CPL_BIT(data, bit)			((data) ^= ((0x01u << (bit))))#define GET_BIT(data, bit)			(((data) & (0x01u << (bit))) == (0x01u << (bit)))#define SET_BIT_EX(data, bit)		SET_BIT(data[(bit) / (sizeof(data[0]) * 8)], ((bit) % (sizeof(data[0]) * 8)))#define CLR_BIT_EX(data, bit)		CLR_BIT(data[(bit) / (sizeof(data[0]) * 8)], ((bit) % (sizeof(data[0]) * 8)))#define CPL_BIT_EX(data, bit)		CPL_BIT(data[(bit) / (sizeof(data[0]) * 8)], ((bit) % (sizeof(data[0]) * 8)))#define GET_BIT_EX(data, bit)		GET_BIT(data[(bit) / (sizeof(data[0]) * 8)], ((bit) % (sizeof(data[0]) * 8)))/* *定义数组长度宏 */#define ARRAYLEN(array)				(sizeof(array) / sizeof(array[0]))#define OFFSETOF(s,m)				((int)&(((s *)NULL)->m))#define SIZEOFFIELD(s,m)			sizeof(((s *)NULL)->m)#if 0typedef char INT8;typedef short INT16;typedef int INT32;typedef long LONG;typedef double long DLONG;typedef unsigned char UINT8;typedef unsigned short UINT16;typedef unsigned int UINT32;typedef unsigned char UCHAR;typedef unsigned short USHORT;typedef unsigned int UINT;typedef unsigned long ULONG;typedef int BOOL;typedef int STATUS;typedef int ARGINT;typedef const char CINT8;#endiftypedef union _BYTE_VAL{
	INT8 Val;
	struct	{
		UINT8 b0:1;
		UINT8 b1:1;
		UINT8 b2:1;
		UINT8 b3:1;
		UINT8 b4:1;
		UINT8 b5:1;
		UINT8 b6:1;
		UINT8 b7:1;
	}bits;
}BYTE_VAL, BYTE_BITS;
typedef union _WORD_VAL{	INT16 Val;
	struct	{
		UINT8 b0:1;
		UINT8 b1:1;
		UINT8 b2:1;
		UINT8 b3:1;
		UINT8 b4:1;
		UINT8 b5:1;
		UINT8 b6:1;
		UINT8 b7:1;
		UINT8 b8:1;
		UINT8 b9:1;
		UINT8 b10:1;
		UINT8 b11:1;
		UINT8 b12:1;
		UINT8 b13:1;
		UINT8 b14:1;
		UINT8 b15:1;
	}bits;
}WORD_VAL, WORD_BITS;

/* */#define FOREVER for (;;)#ifndef MAX#define MAX(x, y)   (((x) < (y)) ? (y) : (x))#endif/* */#ifndef MIN#define MIN(x, y)   (((x) < (y)) ? (x) : (y))#endif/* */#define ISASCII(C)  ((unsigned) (c) <= 0177)#define TOASCII(C)  ((c) & 0177)
#define DEBUGLOG printf#endif							/* __INC_TYPDEF */
