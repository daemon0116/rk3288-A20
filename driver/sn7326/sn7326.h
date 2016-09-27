/*
 * Analog Devices SN7326 I/O Expander and QWERTY Keypad Controller
 *
 * Copyright 2009-2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef _SN7326_H
#define _SN7326_H
#define SN7326_DRIVER_VERSION   "V1.0<2016/09/13>"
#define SN7326_DRV_NAME         "sn7326"
#define I2C_NAME_SIZE           20

#define I2C_SN7326_MAJOR        125
/*
 * gpio 引脚定义
 * PA:0 PB:24 PC:54 PD:85 PE:119 PF:137 PG:149 PH:167 PI:201]
 */
//#define SN7326_SCK      42//PB18 24+18
//#define SN7326_SDA      43//PB19 24+19
//#define SN7326_INT      187//PH20 167+20
//#define SN7326_RST      37//PB13 24+13
/* sn7326 slave address */
#define SN7326_ADDR         0x58
/* register operator address */
#define SN7326_CONFIG_REG   0x08
#define SN7326_STATE_REG    0x10

/*
 * Configuration Register SN7326_CONFIG_REG
 *  D7      D6      D5      D4      D3      D2      D1      D0
 *Reserve   ACI:1   ACI:0   DE      SD      LE      LT:1    LT:0
 * ACI:自动清除中断 00:无自动清除 01:5ms自动清除 10:10ms自动清除 默认:0
 * DE:输入端口去抖使能位 0:关闭 1:开启 默认:开启
 * SD:去抖时间 0:6-8ms 1:3-4ms  默认:0
 * LE:检测长按键使能 0:关闭 1:开启  默认:关闭
 * LT:长按键延时位 00:20ms 01:40ms 10:1s 11:2s  默认:00
 */
#define SN7326_ACI_BIT      0x05
#define SN7326_DE_BIT       0x04
#define SN7326_SD_BIT       0x03
#define SN7326_LE_BIT       0x02
#define SN7326_LT_BIT       0x00
/*
 *KeyPad State Register SN7326_STATE_REG
 *  D7      D6      D5-D0
 *  DN      KS      KM
 *  DN:按键个位数 0:1个 1:多个 默认:1个
 *  KS:按键状态位 0:松开 1:按下 默认:松开
 *  KM:按键位编码64=8x8
 */
#define SN7326_DN_BIT       0x07
#define SN7326_KS_BIT       0x06
#define SN7326_KM_BIT       0x00
#define SN7326_KEY_MASK     0x1F
/*---------------------------------
 *          PP0     PP1     PP2    | PP3     PP4     PP5     PP6     PP7
 * OD0      000000  000001  000010 | 000011 000100  000101  000110  000111
 * OD1      001000  001001  001010 | 001011 001100  001101  001110  001111
 * OD2      010000  010001  010010 | 010011 010100  010101  010110  010111
 * OD3      011000  011001  011010 | 011011 011100  011101  011110  011111
  ----------------------------------
 * OD4      100000  100001  100010 | 100011 100100  100101  100110  100111
 * OD5      101000  101001  101010  101011  101100  101101  101110  101111
 * OD6      110000  110001  110010  110011  110100  110101  110110  110111
 * OD7      111000  111001  111010  111011  111100  111101  111110  111111
 */
#define NUMBER_ONE      0x00
#define NUMBER_TWO      0x08
#define NUMBER_THREE    0x10

#define NUMBER_FOUR     0x18
#define NUMBER_FIVE     0x01
#define NUMBER_SIX      0x09

#define NUMBER_SEVEN    0x11
#define NUMBER_EIGHT    0x19
#define NUMBER_NINE     0x02

#define NUMBER_SURE     0x0A
#define NUMBER_ZERO     0x12
#define NUMBER_CANCEL   0x1A

struct i2c_client; /* forward declaration */
#endif
