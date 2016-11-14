/* $Id: wavfile.c,v 1.2 2011/06/09 00:51:02 ve3wwg Exp $
 * Copyright: wavfile.c (c) Erik de Castro Lopo  erikd@zip.com.au
 *
 * wavfile.c - Functions for reading and writing MS-Windoze .WAV files.
 *
 * This  program is free software; you can redistribute it and/or modify it
 * under the  terms  of  the GNU General Public License as published by the
 * Free Software Foundation.
 * 
 * This  program  is  distributed  in  the hope that it will be useful, but
 * WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty  of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details (licensed by file COPYING or GPLv*).
 * 
 * This code was originally written to manipulate Windoze .WAV files
 * under i386 Linux (erikd@zip.com.au).
 *
 * ve3wwg@gmail.com
 */	
static const char rcsid[] = "$Id: wavfile.c,v 1.2 2011/06/09 00:51:02 ve3wwg Exp $";

#include  	<stdio.h>
#include	<errno.h>
#include	<sys/types.h>
#include    <stdint.h>
#include	<unistd.h>
#include  	<string.h>
#include	"wavfile.h"


#define		BUFFERSIZE   		128
#define		PCM_WAVE_FORMAT   	1
#define		TRUE				1
#define		FALSE				0

typedef  struct	{
	uint32_t	dwSize;
	uint16_t	wFormatTag;
	uint16_t	wChannels;
	uint32_t	dwSamplesPerSec;
	uint32_t	dwAvgBytesPerSec;
	uint16_t	wBlockAlign;
	uint16_t	wBitsPerSample;
} WAVEFORMAT;

typedef  struct	{
	char    	RiffID [4];
	uint32_t    	RiffSize;
	char    	WaveID[4];
	char    	FmtID[4];
	uint32_t    	FmtSize;
	uint16_t   	wFormatTag;
	uint16_t   	nChannels;
	uint32_t	nSamplesPerSec;
	uint32_t	nAvgBytesPerSec;
	uint16_t	nBlockAlign;
	uint16_t	wBitsPerSample;
	char		DataID[4];
	uint32_t	nDataBytes;
} WAVE_HEADER;

static  WAVE_HEADER  waveheader = {
	{ 'R', 'I', 'F', 'F' },
	0,
	{ 'W', 'A', 'V', 'E' },
	{ 'f', 'm', 't', ' ' },
	16,				/* FmtSize*/
	PCM_WAVE_FORMAT,		/* wFormatTag*/
	0,				/* nChannels*/
	0,
	0,
	0,
	0,
	{ 'd', 'a', 't', 'a' },
	0
}; /* waveheader*/


static char *findchunk(char *pstart,const char *fourcc,size_t n);

/*
 * Error reporting function for this source module:
 */
static void
err(const char *format,...) {
//	va_list ap;

//	if ( v_erf == NULL )
//		return;				/* Only report error if we have function */
//	va_start(ap,format);
//	v_erf(format,ap);			/* Use caller's supplied function */
//	va_end(ap);
}

int WaveWriteHeader(int wavefile,int channels,uint32_t samplerate,int sampbits,uint32_t samples) {
 	uint32_t databytes;
	uint16_t blockalign;


	if ( wavefile < 0 ) {
		err("Invalid file descriptor");
		return WW_BADOUTPUTFILE;
	}

	sampbits   = (sampbits == 16) ? 16 : 8;

	blockalign = ((sampbits == 16) ? 2 : 1) * channels;
	databytes  = samples * (uint32_t) blockalign;

	waveheader.RiffSize 	   = sizeof (WAVE_HEADER) + databytes - 8;
	waveheader.wFormatTag      = PCM_WAVE_FORMAT;
	waveheader.nChannels       = channels;
	waveheader.nSamplesPerSec  = samplerate;
	waveheader.nAvgBytesPerSec = samplerate * (uint32_t) blockalign;
	waveheader.nBlockAlign     = blockalign;
	waveheader.wBitsPerSample  = sampbits;
	waveheader.nDataBytes      = databytes;

	if ( write(wavefile,&waveheader,sizeof (WAVE_HEADER)) != sizeof (WAVE_HEADER) ) {
		err("%s",strerror(errno));	/* wwg: report the error */
		return  WW_BADWRITEHEADER;
	}

  return 0;
} /* WaveWriteHeader*/

int WaveReadHeader(int wavefile, pPCM_PARAM pcm, uint32_t *pdatastart) {
	WAVEFORMAT  waveformat;
	char buffer[BUFFERSIZE];		/* Function is not reentrant.*/
	char*   ptr;
	uint32_t databytes;
    uint32_t datastart;

	if ( lseek(wavefile,0L,SEEK_SET) ) {
		err("%s",strerror(errno));		/* wwg: Report error */
		return  WR_BADSEEK;
	}

	read(wavefile,buffer,BUFFERSIZE);

	if ( findchunk(buffer,"RIFF",BUFFERSIZE) != buffer ) {
		err("Bad format: Cannot find RIFF file marker");	/* wwg: Report error */
		return  WR_BADRIFF;
	}

	if ( !findchunk(buffer,"WAVE",BUFFERSIZE) ) {
		err("Bad format: Cannot find WAVE file marker");	/* wwg: report error */
		return  WR_BADWAVE;
	}

	ptr = findchunk(buffer,"fmt ",BUFFERSIZE);

	if ( !ptr ) {
		err("Bad format: Cannot find 'fmt' file marker");	/* wwg: report error */
		return  WR_BADFORMAT;
	}

	ptr += 4;	/* Move past "fmt ".*/
	memcpy(&waveformat,ptr,sizeof (WAVEFORMAT));

 	if ( waveformat.dwSize < (sizeof (WAVEFORMAT) - sizeof (uint32_t)) ) {
		err("Bad format: Bad fmt size");			/* wwg: report error */
		return  WR_BADFORMATSIZE;
	}

	if ( waveformat.wFormatTag != PCM_WAVE_FORMAT ) {
		err("Only supports PCM wave format");			/* wwg: report error */
		return  WR_NOTPCMFORMAT;
	}

	ptr = findchunk(buffer,"data",BUFFERSIZE);

	if ( !ptr ) {
		err("Bad format: unable to find 'data' file marker");	/* wwg: report error */
		return  WR_NODATACHUNK;
	}

	ptr += 4;	/* Move past "data".*/
	memcpy(&databytes,ptr,sizeof (uint32_t));

	/* Everything is now cool, so fill in output data.*/

	pcm->channels   = waveformat.wChannels;
	pcm->rate = waveformat.dwSamplesPerSec;
	pcm->bitsPerSample = waveformat.wBitsPerSample;

	datastart  = (uint32_t) ( (ptr + 4) - buffer );

	if ( waveformat.dwSamplesPerSec != waveformat.dwAvgBytesPerSec / waveformat.wBlockAlign ) {
		err("Bad file format");			/* wwg: report error */
		return  WR_BADFORMATDATA;
	}

	if ( waveformat.dwSamplesPerSec != waveformat.dwAvgBytesPerSec / waveformat.wChannels / ((waveformat.wBitsPerSample == 16) ? 2 : 1) ) {
		err("Bad file format");			/* wwg: report error */
		return  WR_BADFORMATDATA;
	}

    lseek(wavefile, datastart, SEEK_SET);

    if(pdatastart){
        *pdatastart = datastart;
    }

	return  0;

} /* WaveReadHeader*/

/*********************************************************************
 * Find a Chunk
 *********************************************************************/
static char *
findchunk(char *pstart,const char *fourcc,size_t n) {
	char	*pend;
	int	k, test;

	pend = pstart + n;

	while ( pstart < pend ) {
	 	if ( *pstart == *fourcc) { 	/* found match for first char*/
			test = TRUE;
			for ( k = 1; fourcc[k] != 0; k++ )
				test = (test ? ( pstart [k] == fourcc [k] ) : FALSE);
			if ( test )
				return  pstart;
			} /* if*/
		pstart++;
		} /* while lpstart*/

	return  NULL;
} /* findchuck*/

/* $Source: /cvsroot/wavplay/code/src/wavfile.c,v $ */
