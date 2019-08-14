#include "server.h"
#include "csapp.h"
#include "transaction.h"
#include "protocol.h"
#include "data.h"
#include "store.h"
#include <time.h>
#include "debug.h"
CLIENT_REGISTRY *client_registry;
BLOB *blob;
KEY *key;
BLOB *value;
long int ns;
uint64_t all;
time_t sec;
struct timespec spec;
void *xacto_client_service(void *arg){
    int client_connfd = *((int *)arg);
    Free(arg);
    Pthread_detach(Pthread_self());
    creg_register(client_registry,client_connfd);
    TRANSACTION *new_transaction = trans_create();;
    XACTO_PACKET *pkt = Malloc(sizeof(XACTO_PACKET));
    void *data= NULL;
    void *temp_key = NULL;
    void *val = NULL;
    BLOB *get_val = NULL;
    while(1){
        debug("going to hit recv packet");
        store_show();
        trans_show_all();
        if(proto_recv_packet(client_connfd, pkt, &data)==-1){
            if(data)
                Free(data);
            trans_abort(new_transaction);
            break;
        }
        debug("received pakt");
        if(pkt->type == XACTO_PUT_PKT){
            debug("PUT");
            if(proto_recv_packet(client_connfd, pkt, &temp_key)==-1){
                if(temp_key)
                    Free(temp_key);
                trans_abort(new_transaction);
                break;
            }
            size_t key_size = pkt->size;
            if(proto_recv_packet(client_connfd,pkt,&val)==-1){
                if(val)
                    Free(val);
                trans_abort(new_transaction);
                break;
            }
            size_t val_size = pkt->size;
            blob = blob_create((char*)temp_key,key_size);
            key = key_create(blob);
            value = blob_create((char*)val,val_size);
            pkt->status = store_put(new_transaction,key, value);
            pkt->type = XACTO_REPLY_PKT;
            pkt->null =0;
            pkt->size =0;
            clock_gettime(CLOCK_MONOTONIC, &spec);
            pkt->timestamp_sec = spec.tv_sec;
            pkt->timestamp_nsec = spec.tv_nsec;
            proto_send_packet(client_connfd, pkt, NULL);
            store_show();
            Free(temp_key);
            Free(val);
            if(data)
                Free(data);
            continue;
        }
        else if(pkt->type == XACTO_GET_PKT){
            if(proto_recv_packet(client_connfd, pkt, &temp_key)==-1){
                if(temp_key)
                    Free(temp_key);
                trans_abort(new_transaction);
                break;
            }
            blob = blob_create((char*)temp_key,pkt->size);
            key = key_create(blob);
            pkt->status = store_get(new_transaction,key,&get_val);
            pkt->type = XACTO_REPLY_PKT;
            pkt->null = 1;
            pkt->size = 0;
            clock_gettime(CLOCK_MONOTONIC, &spec);
            pkt->timestamp_sec = spec.tv_sec;
            pkt->timestamp_nsec = spec.tv_nsec;
            proto_send_packet(client_connfd, pkt, NULL);
            if(get_val!=NULL){
                pkt->type = XACTO_DATA_PKT;
                pkt->null = 0;
                pkt->size = strlen(get_val->prefix);
                clock_gettime(CLOCK_MONOTONIC, &spec);
                pkt->timestamp_sec = spec.tv_sec;
                pkt->timestamp_nsec = spec.tv_nsec;
                proto_send_packet(client_connfd, pkt,get_val->prefix);
                //Free(temp_key);
            }
            else{
                pkt->type = XACTO_DATA_PKT;
                pkt->null = 1;
                pkt->size = 0;
                clock_gettime(CLOCK_MONOTONIC, &spec);
                pkt->timestamp_sec = spec.tv_sec;
                pkt->timestamp_nsec = spec.tv_nsec;
                proto_send_packet(client_connfd, pkt,NULL);
                //Free(temp_key);
            }

            Free(temp_key);
            if(data)
                Free(data);
            continue;
        }
        else if(pkt->type == XACTO_COMMIT_PKT){
            pkt->type = XACTO_REPLY_PKT;
            pkt->null = 1;
            pkt->size = 0;
            clock_gettime(CLOCK_MONOTONIC, &spec);
            pkt->timestamp_sec = spec.tv_sec;
            pkt->timestamp_nsec = spec.tv_nsec;
            pkt->status = trans_commit(new_transaction);
            proto_send_packet(client_connfd, pkt, NULL);
            if(data)
                Free(data);
            //Free(temp_key);
            break;
        }
    }
    //Free(data);
    Free(pkt);

    Close(client_connfd);
    debug("calline client unregister");
    creg_unregister(client_registry, client_connfd);
    Pthread_exit(0);
    return NULL;
}