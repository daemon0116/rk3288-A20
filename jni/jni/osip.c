
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>



#include "osip2/osip_mt.h"
#include "eXosip2/eXosip.h"
#include "eXosip2/eX_setup.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#if 0
//struct eXosip_t excontext;

void *osip_recv(void *p)
{
    pthread_detach(pthread_self());

    
    eXosip_event_t *je = NULL;
    osip_message_t *ack = NULL;
    osip_message_t *invite = NULL;
    osip_message_t *answer = NULL;
    sdp_message_t *remote_sdp = NULL;
    int call_id, dialog_id;
    int i,j;
    int id;

    for(;;)
    {
        // 侦听是否有消息到来
        je = eXosip_event_wait (0, 50); 

        // 协议栈带有此语句,具体作用未知
        eXosip_lock ();
        eXosip_default_action (je);
        eXosip_automatic_refresh ();
        eXosip_unlock ();

        if (je == NULL) // 没有接收到消息，继续
        {
            continue;
        }

        switch (je->type)
        {
        case EXOSIP_MESSAGE_NEW: // 新的消息到来
            printf("\n\t*** EXOSIP_MESSAGE_NEW!\n");

            if (MSG_IS_MESSAGE (je->request)) // 如果接收到的消息类型是 MESSAGE
            {
                {
                    osip_body_t *body;
                    osip_message_get_body (je->request, 0, &body); 
                    printf("I get the msg is: %s", body->body);
                }

                // 按照规则，需要回复 OK 信息
                eXosip_message_build_answer (je->tid, 200, &answer);
                eXosip_message_send_answer (je->tid, 200, answer);
            }
            break;


        case EXOSIP_CALL_ACK:
            printf("\n\t--> ACK recieved!\n");
            // printf ("the cid is %s, did is %s\n", je->did, je->cid); 
            break;

        case EXOSIP_CALL_CLOSED:
            printf("\n\t--> the remote hold the session!\n");
            // eXosip_call_build_ack(dialog_id, &ack);
            // eXosip_call_send_ack(dialog_id, ack); 
            i = eXosip_call_build_answer (je->tid, 200, &answer);
            if (i != 0)
            {
                printf ("This request msg is invalid!Cann't response!\n");
                eXosip_call_send_answer (je->tid, 400, NULL);
            }
            else
            {
                eXosip_call_send_answer (je->tid, 200, answer);
                printf("\n\t--> bye send 200 over!\n");
            } 
            break;

        case EXOSIP_CALL_MESSAGE_NEW:

            printf("\n\t*** EXOSIP_CALL_MESSAGE_NEW\n");
            if (MSG_IS_INFO(je->request) ) // 如果传输的是 INFO 方法
            {
                eXosip_lock ();
                i = eXosip_call_build_answer (je->tid, 200, &answer);
                if (i == 0)
                {
                    eXosip_call_send_answer (je->tid, 200, answer);
                }

                eXosip_unlock ();

                {
                    osip_body_t *body;
                    osip_message_get_body (je->request, 0, &body);
                    printf("the body is %s", body->body);
                }
            }
            break; 

        default:
            printf("\n\t--> Could not parse the msg: %d!\n", je->type);
        }
    } 
}
#endif

#if 1

int osip_test(int argc, char* argv[])
{
    eXosip_event_t *je;
    osip_message_t *reg = NULL;
    osip_message_t *invite = NULL;
    osip_message_t *ack = NULL;
    osip_message_t *info = NULL;
    osip_message_t *message = NULL;
    int call_id, dialog_id;
    int i,flag;
    int flag1 = 1;
    int id;
    
    char *strSrcCall =  "sip:192.168.80.234:6000";
    char *strDestCall = "sip:192.168.80.222:6000";
    //char *strDestCall = "sip:225.0.0.2:6000";


    char command;
    char tmp[4096];
    char localip[128];

    char msg[] = "Hello world, I love the world!";

#if 0


    i = eXosip_init();
    if (i != 0)
    {
        printf("\t--> Couldn't initialize eXosip! <--\n");
        return -1;
    }
    else
    {
        printf("\t--> eXosip_init successfully! <-- \n\n");
    }





    i = eXosip_listen_addr (IPPROTO_UDP, NULL, 6000, AF_INET, 0);
    if (i != 0)
    {
        eXosip_quit ();
        printf("\n\t--> Couldn't initialize transport layer! <-- \n\n");
        return -1;
    }

    _pthread_t thread;

    pthread_create(&thread, NULL, osip_recv, NULL);

    //while(1)sleep(2);
        
    while(1)
    {
        // 传输 MESSAGE方法,也就是即时消息，
        // 和 INFO 方法相比，主要区别，是 MESSAGE 不用建立连接，直接传输信息，
        // 而 INFO 必须在建立 INVITE 的基础上传输。

        sleep(2);

        #if 1
        printf("\n\t--> the mothed :MESSAGE \n");
        eXosip_message_build_request (&message, 
                                      "MESSAGE", 
                                      strDestCall, 
                                      strSrcCall, 
                                      NULL);
        osip_message_set_body (message, msg, sizeof(msg));
        
        // 假设格式是xml
        osip_message_set_content_type (message, "text/plain");
        eXosip_message_send_request (message);
        #endif
    }
#endif
    return 0;
}

#endif
