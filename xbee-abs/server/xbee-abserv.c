#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <propeller.h>
#include "xbeeframe.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define OPTION_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Access-Control-Allow-Methods: GET, POST, OPTIONS, XPING, XABS\r\n\
Access-Control-Allow-Headers: XPING, XABS\r\n\
Access-Control-Max-Age: 1000000\r\n\
Keep-Alive: timeout=1, max=100\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n"

#define CANNED_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 11\r\n\
\r\n\
Got request"

#define XPING_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 6\r\n\
\r\n\
Hello!"

/* Xbee pins */
#define XBEE_RX     13
#define XBEE_TX     12
#define XBEE_BAUD   9600

#define ID_IPV4TX   0x20
#define ID_IPV4RX   0xb0

typedef struct {
    uint8_t apiid;
    uint8_t frameid;
    uint8_t dstaddr[4];
    uint8_t dstport[2];
    uint8_t srcport[2];
    uint8_t protocol;
    uint8_t options;
    uint8_t data[1];
} IPV4TX_header;

typedef struct {
    uint8_t apiid;
    uint8_t srcaddr[4];
    uint8_t dstport[2];
    uint8_t srcport[2];
    uint8_t protocol;
    uint8_t status;
    uint8_t data[1];
} IPV4RX_header;

#if 0
/* list of drivers we can use */
extern _Driver _FullDuplexSerialDriver;
_Driver *_driverlist[] = {
    &_FullDuplexSerialDriver,
    NULL
};
#endif

void handle_ipv4_frame(XbeeFrame_t *mbox, uint8_t *frame, int length);
void show_frame(uint8_t *frame, int length);

int main(void)
{
    uint8_t frame[1024];
    XbeeFrameInit_t init;
    XbeeFrame_t mailbox;
    
    printf("Starting frame driver\n");
    if (XbeeFrame_start(&init, &mailbox, XBEE_RX, XBEE_TX, 0, XBEE_BAUD) < 0) {
        printf("failed to start frame driver\n");
        return 1;
    }
    
    printf("Getting our IP address\n");
    frame[0] = 0x08;
    frame[1] = 0x01;
    frame[2] = 'M';
    frame[3] = 'Y';
    XbeeFrame_sendframe(&mailbox, frame, 4);
    
    printf("Listening for frames\n");
    while (1) {
        uint8_t *frame;
        int length;
        if ((frame = XbeeFrame_recvframe(&mailbox, &length)) != NULL) {
        
            printf("[RX]");
            //show_frame(frame, length);
            
            /* handle IPv4 packets received from the Xbee */
            if (frame[0] == ID_IPV4RX)
                handle_ipv4_frame(&mailbox, frame, length);
                
            XbeeFrame_release(&mailbox);
        }
    }
    
    return 0;
}

uint8_t *pkt_ptr;
int pkt_len;

int match(char *str)
{
    uint8_t *ptr = pkt_ptr;
    int len = pkt_len;
    while (*str) {
        if (--len < 0 || *ptr++ != *str++)
            return FALSE;
    }
    pkt_ptr = ptr;
    pkt_len = len;
    return TRUE;
}

void skip_spaces(void)
{
    while (pkt_len > 0 && *pkt_ptr == ' ') {
        --pkt_len;
        ++pkt_ptr;
    }
}

void  skip_white(void)
{
    while (pkt_len > 0 && isspace(*pkt_ptr)) {
        --pkt_len;
        ++pkt_ptr;
    }
}

char *find(char *str)
{
    char *s = strstr((char*) pkt_ptr, str);
    if(s && s > (char*) pkt_ptr) {
        pkt_len -= s - (char*) pkt_ptr;
        pkt_ptr = (uint8_t*)s;
        return s;
    }
    return 0;
}

int token(char *str)
{
    int n = 0;
    char *s = (char*)pkt_ptr;
    skip_white();
    s = (char*)pkt_ptr;
    while(pkt_len > 0 && !isspace(*s)) {
        str[n] = *(s++);
        n++;
    }
    str[n] = 0;
    pkt_len -= n;
    pkt_ptr = (uint8_t*)s;
    return n;
}

void send_response(XbeeFrame_t *mbox, IPV4RX_header *rxhdr, uint8_t *data, int length)
{
    uint8_t txframe[1024];
    IPV4TX_header *txhdr = (IPV4TX_header *)txframe;
    
    txhdr->apiid = ID_IPV4TX;
    txhdr->frameid = 0x42;
    memcpy(&txhdr->dstaddr, &rxhdr->srcaddr, 4);
    memcpy(&txhdr->dstport, &rxhdr->srcport, 2);
    memcpy(&txhdr->srcport, &rxhdr->dstport, 2);
    txhdr->protocol = rxhdr->protocol;
    txhdr->options = 0x00; // don't terminate after send
    //txhdr->options = 0x01; // terminate after send
    memcpy(txhdr->data, data, length);
    length += sizeof(IPV4TX_header) - 1;

    printf("[TX]");
    show_frame(txframe, length);

    XbeeFrame_sendframe(mbox, txframe, length);
}

void show_frame(uint8_t *frame, int length)
{
    int i;
    for (i = 0; i < length; ++i)
        printf(" %02x", frame[i]);
    printf("\n     \"");
    for (i = 0; i < length; ++i) {
        if (frame[i] >= 0x20 && frame[i] <= 0x7e)
            putchar(frame[i]);
        else
            printf("<%02x>", frame[i]);
        if (frame[i] == '\n')
            putchar('\n');
    }
    printf("\"\n");
}

void handle_ipv4_frame(XbeeFrame_t *mbox, uint8_t *frame, int length)
{
    int  num = 0;
    char str[80];
    pkt_ptr = frame + (int)&((IPV4RX_header *)0)->data;
    pkt_len = length;
    
    if (match("XABS"))
    {
        if(find("\r\n\r\n"))
        { // got message body
            skip_white();

            //
            // COMMAND: XABS PIN # [HIGH|LOW|TOGGLE|INPUT]
            //
            if (match("PIN"))
            { // got pin
                skip_spaces();
                if(token(str)) {
                    num = atoi(str);
                }
                skip_spaces();
                if (match("HIGH")) {
                    DIRA |= 1 << num;
                    OUTA |= 1 << num;
                }
                else if (match("LOW")) {
                    DIRA |= 1 << num;
                    OUTA &= ~(1 << num);
                }
                else if (match("TOGGLE")) {
                    DIRA |= 1 << num;
                    OUTA ^= 1 << num;
                }
                else if (match("INPUT")) {
                    DIRA &= ~(1 << num);
                }
            }
        }
        send_response(mbox, (IPV4RX_header *)frame, (uint8_t *)CANNED_RESPONSE, sizeof(CANNED_RESPONSE) - 1);
    }
    else if (match("XPING")) {
        send_response(mbox, (IPV4RX_header *)frame, (uint8_t *)XPING_RESPONSE, sizeof(XPING_RESPONSE) - 1);
    }
    else if (match("OPTIONS")) {
        send_response(mbox, (IPV4RX_header *)frame, (uint8_t *)OPTION_RESPONSE, sizeof(OPTION_RESPONSE) - 1);
    }
    else {
        // bad request
    }
}
