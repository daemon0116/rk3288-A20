/*****************************************************************

******************************************************************/

#include "log.h"

#include "NetWork.h"
#include "sip.h"


_lock_t SipLock;


const char sip_method_MESSAGE[]  = "MESSAGE";
const char sip_method_REGISTER[] = "REGISTER";
const char sip_method_OPTIONS[]  = "OPTIONS";
const char sip_method_NOTIFY[]   = "NOTIFY";
const char sip_method_INVITE[]   = "INVITE";
const char sip_method_BYE[]      = "BYE";

const char sip_state_OK[] = "200 OK";


const char sip_header_Via[] = "Via";
const char sip_header_From[] = "From";
const char sip_header_To[] = "To";
const char sip_header_Call_ID[] = "Call-ID";
const char sip_header_CSeq[] = "CSeq";
const char sip_header_Content_Type[] = "Content";
const char sip_header_Expires[] = "Expires";
const char sip_header_Allow[] = "Allow";
const char sip_header_Max_Forwards[] = "Max-Forwards";
const char sip_header_Contact[] = "Contact";
const char sip_header_Unauthorized[] = "Unauthorized";



const char sip_line_MESSAGE[] = "MESSAGE sip:%s:%d SIP/2.0\r\n";
const char sip_line_Via[] = "Via: SIP/2.0/UDP %s:%d;rport;branch=z9hG4bK-%d\r\n";
const char sip_line_From[] = "From: <sip:%s:%d>;tag=676238418\r\n";
const char sip_line_To[] = "To: <sip:%s:%d>\r\n";
const char sip_line_Call_ID[] = "Call-ID: 834401259\r\n";
const char sip_line_CSeq[] = "CSeq: 20 MESSAGE\r\n";
const char sip_line_Content_Type[] = "Content-Type: text/plain\r\n";
const char sip_line_Max_Forwards[] = "Max-Forwards: 70\r\n";
const char sip_line_User_Agent[] = "User-Agent: nvis\r\n";
const char sip_line_Content_Length[] = "Content-Length:%d\r\n\r\n";
const char sip_line_Ok[] = "SIP/2.0 200 OK\r\n";


int sip_get_line(char *obuf, int size, const char *ibuf, const char *header)
{
    char *p1;
    char *p2;
    int  len;
    
    if(!(p1 = strstr(ibuf, header))){
        return 0;
    }

    if(!(p2 = strstr(p1, "\r\n"))){
        return 0;
    }

    len = p2-p1+2; //+2是\r\n
    if(len >= size){
        len = size-1;
    }

    memcpy(obuf, p1, len);
    obuf[len] = 0;

    return len;
}

int sip_make_msg(const char *to, uint16 port, const char *data, char *out, int size, int *psn)
{
    int len = 0;
    static int sn = 1;
    int sn_tmp;

    _lock(&SipLock);
    sn_tmp = sn++;
    _unlock(&SipLock);

    len += snprintf(out+len, size-len, sip_line_MESSAGE, to, port);
    len += snprintf(out+len, size-len, sip_line_Via, pSysInfo->IP, PUBLIC_GROUP_PORT, sn_tmp);
    len += snprintf(out+len, size-len, sip_line_From, pSysInfo->IP, PUBLIC_GROUP_PORT);
    len += snprintf(out+len, size-len, sip_line_To, to, port );
    len += snprintf(out+len, size-len, sip_line_Call_ID);
    len += snprintf(out+len, size-len, sip_line_CSeq);
    len += snprintf(out+len, size-len, sip_line_Content_Type);
    len += snprintf(out+len, size-len, sip_line_Max_Forwards);
    len += snprintf(out+len, size-len, sip_line_User_Agent);
    len += snprintf(out+len, size-len, sip_line_Content_Length, strlen(data));
    len += snprintf(out+len, size-len, "%s", data);

    if(psn){
        *psn = sn_tmp;
    }
    return len;
}


int sip_make_ok(const char *pmsg, const char *data, char *out, int size)
{
    int len;

    len += snprintf(out, size-len, sip_line_Ok);
    len += sip_get_line(out+len, size-len, pmsg, sip_header_Via);
    len += sip_get_line(out+len, size-len, pmsg, sip_header_From);
    len += sip_get_line(out+len, size-len, pmsg, sip_header_To);
    len += sip_get_line(out+len, size-len, pmsg, sip_header_Call_ID);
    len += sip_get_line(out+len, size-len, pmsg, sip_header_CSeq);
    len += sip_get_line(out+len, size-len, pmsg, sip_header_Content_Type);
    len += sip_get_line(out+len, size-len, pmsg, sip_header_Max_Forwards);
    len += snprintf(out+len, size-len, sip_line_Content_Length, strlen(data));
    len += snprintf(out+len, size-len, "%s", data);

    return len;
}

int sip_get_msg_body(const char *data, char *out, int size)
{
    const char *p;
    
    if(!strstr(data, sip_method_MESSAGE)){
        return 0;
    }

    if(!(p = strstr(data, "\r\n\r\n"))){
        return 0;
    }
    
    p += 4; //跳过"\r\n\r\n"

    strncpy(out, p, size);
    
    return strlen(p);
}

const char *sip_find_msg_body(const char *pmsg)
{
    const char *p;
    
    if(!strstr(pmsg, sip_method_MESSAGE)){
        return NULL;
    }

    if(!(p = strstr(pmsg, "\r\n\r\n"))){
        return NULL;
    }
    
    p += 4; //跳过"\r\n\r\n"

    return (p);
}

const char *sip_find_ok_body(const char *pmsg)
{
    const char *p;

    if(!strstr(pmsg, sip_state_OK)){
        return NULL;
    }

    if(!(p = strstr(pmsg, "\r\n\r\n"))){
        return NULL;
    }

    p += 4; //跳过"\r\n\r\n"

    return (p);
}

const char *sip_get_ip(const char *key, const char *data, char *out, int size)
{
    const char *start;
    const char *end;
    
    if(!(start = strstr(data, key))){
        return NULL;
    }
    if(!(start = strchr(start, '<'))){
        return NULL;
    }
    if(!(start = strchr(start, ':'))){
        return NULL;
    }
    start ++;
    if(!(end = strchr(start, ':'))){
        return NULL;
    }

    int len = end-start>=size ? size-1:end-start;
    memcpy(out, start, len);
    out[len] = 0;

    return out;
}

uint16 sip_get_port(const char *key, const char *data)
{
    const char *start;
    const char *end;
    char value[6];
    
    if(!(start = strstr(data, key))){
        return 0;
    }
    if(!(start = strchr(start, '.'))){
        return 0;
    }
    if(!(start = strchr(start, ':'))){
        return 0;
    }
    start ++;
    if(!(end = strchr(start, '>'))){
        return 0;
    }

    int size = end-start;
    if(size >= sizeof(value)){
        return 0;
    }
    memcpy(value, start, size);
    value[size] = 0;

    return atoi(value);
}


const char *sip_find_ok(const char *pmsg)
{
    const char *p;
    
    p = strstr(pmsg, sip_state_OK);

    return (p);
}




const char *get_value_char(const char *key, const char *data, char *out, int size)
{
    const char *start;
    const char *end;
    
    if(!(start = strstr(data, key))){
        return NULL;
    }
    if(!(start = strchr(start, ':'))){
        return NULL;
    }
    if(!(start = strchr(start, '"'))){
        return NULL;
    }
    start ++;
    if(!(end = strchr(start, '"'))){
        return NULL;
    }

    int len = end-start>=size ? size-1:end-start;
    memcpy(out, start, len);
    out[len] = 0;

    return out;
}

const char *get_value_char2(const char *key, const char *data, char *out, int size)
{
    const char *start;
    const char *end;
    const char *pnull = "";
    
    if(!(start = strstr(data, key))){
        return pnull;
    }
    if(!(start = strchr(start, ':'))){
        return pnull;
    }
    if(!(start = strchr(start, '"'))){
        return pnull;
    }
    start ++;
    if(!(end = strchr(start, '"'))){
        return pnull;
    }

    int len = end-start>=size ? size-1:end-start;
    memcpy(out, start, len);
    out[len] = 0;

    return out;
}



int get_value_num(const char *key, const char *data)
{
    const char *start;
    const char *end;
    char value[11];
    
    if(!(start = strstr(data, key))){
        return 0;
    }
    if(!(start = strchr(start, ':'))){
        return 0;
    }
    if(!(start = strchr(start, '"'))){
        return 0;
    }
    start ++;
    if(!(end = strchr(start, '"'))){
        return 0;
    }

    int size = end-start;
    if(size >= sizeof(value)){
        return 0;
    }
    memcpy(value, start, size);
    value[size] = 0;

    return atoi(value);
}


int InitSip(void)
{
    _lock_init(&SipLock, NULL);

    return 0;
}

int DelInitSip(void)
{
    _lock_destroy(&SipLock);

    return 0;
}

