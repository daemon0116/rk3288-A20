/*****************************************************************

******************************************************************/

#ifndef __SIP_H__
#define __SIP_H__

#include "TypepDef.h"

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif


int InitSip(void);
int DelInitSip(void);

int sip_make_msg(const char *to, uint16 port, const char *body, char *pmsg, int size, int *psn);
int sip_make_ok(const char *pmsg, const char *data, char *out, int size);
int sip_get_msg_body(const char *pmsg, char *body, int size);
const char *sip_find_msg_body(const char *pmsg);
const char *sip_find_ok_body(const char *pmsg);
const char *sip_find_ok(const char *pmsg);
const char *sip_get_ip(const char *key, const char *data, char *out, int size);
uint16 sip_get_port(const char *key, const char *data);
const char *get_value_char(const char *key, const char *data, char *out, int size);
const char *get_value_char2(const char *key, const char *data, char *out, int size);
int get_value_num(const char *key, const char *data);


#ifdef __cplusplus
}
#endif

#endif

