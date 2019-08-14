#include "protocol.h"
#include "debug.h"
#include <errno.h>
#include "csapp.h"
int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data){
    uint32_t payload_len = pkt->size;
    pkt->type =pkt->type;
    pkt->null = pkt->null;
    pkt->status = pkt->status;
    pkt->size = htonl(pkt->size);
    pkt->timestamp_sec = htonl(pkt->timestamp_sec);
    pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);
    size_t hdr_size = sizeof(XACTO_PACKET);
    size_t payload_size = payload_len;
    if(rio_writen(fd,pkt,hdr_size)==-1)
        return -1;
    if(payload_len!=0){
        if(rio_writen(fd,data,payload_size)==-1)
            return -1;
    }
    return 0;
}

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap){
    ssize_t hdr_size = sizeof(XACTO_PACKET);
    if(rio_readn(fd,pkt,hdr_size) <= 0){
        debug("EOF on :%d",fd);
        return -1;
    }
    pkt->type =pkt->type;
    pkt->null = pkt->null;
    pkt->status = pkt->status;
    pkt->size = ntohl(pkt->size);
    pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
    pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);
    ssize_t payload_size = (ssize_t)pkt->size;
    debug("payload_size is:%lu",payload_size);
    char* ptr;
    if(pkt->size!=0){
        ptr = Malloc(pkt->size);//Need to free this datap
        if(rio_readn(fd,ptr,payload_size) <= 0){
            debug("EOF on :%d",fd);
            return -1;
        }
        *datap = ptr;
    }
    return 0;
}