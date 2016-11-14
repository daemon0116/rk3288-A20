//md5.h
#ifndef MD5_H
#define MD5_H
#include "TypepDef.h"


struct MD5Context {
        uint32 buf[4];
        uint32 bits[2];
        unsigned char in[64];
};

extern void MD5Init(struct MD5Context *ctx);
extern void MD5Update(struct MD5Context *ctx, char *buf, unsigned len);
extern void MD5Final(unsigned char *digest, struct MD5Context *ctx);
extern void MD5Transform(uint32 *buf, uint32 *in);

/*
* This is needed to make RSAREF happy on some MS-DOS compilers.
*/
typedef struct MD5Context MD5_CTX;

#endif /* !MD5_H */