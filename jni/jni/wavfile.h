/* $Id: wavfile.h,v 1.1 2011/06/01 02:38:45 ve3wwg Exp $
 * Copyright:	wavfile.h (c) Erik de Castro Lopo  erikd@zip.com.au
 *
 * This  program is free software; you can redistribute it and/or modify it
 * under the  terms  of  the GNU General Public License as published by the
 * Free Software Foundation.
 * 
 * This  program  is  distributed  in  the hope that it will be useful, but
 * WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty  of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details (licensed by file COPYING or GPLv*).
 */
#ifndef _wavfile_h
#define _wavfile_h "$Id: wavfile.h,v 1.1 2011/06/01 02:38:45 ve3wwg Exp $"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include "TypepDef.h"

#define WW_BADOUTPUTFILE	1
#define WW_BADWRITEHEADER	2

#define WR_BADALLOC		3
#define WR_BADSEEK		4
#define WR_BADRIFF		5
#define WR_BADWAVE		6
#define WR_BADFORMAT		7
#define WR_BADFORMATSIZE	8

#define WR_NOTPCMFORMAT		9
#define WR_NODATACHUNK		10
#define WR_BADFORMATDATA	11


typedef struct{
    int rate;
    int channels;
    int bitsPerSample;
}PCM_PARAM, *pPCM_PARAM;


extern int WaveWriteHeader(int wavefile,int channels,uint32_t samplerate,int sampbits,uint32_t samples);
extern int WaveReadHeader(int wavefile, pPCM_PARAM pcm, uint32_t *datastart);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _wavfile_h_ */

/* $Source: /cvsroot/wavplay/code/include/wavfile.h,v $ */
