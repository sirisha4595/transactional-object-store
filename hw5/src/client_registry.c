#include "client_registry.h"
#include "csapp.h"
#include "debug.h"
#include <stdio.h>
pthread_mutex_t temp_mutex;
typedef struct client_registry
{
    int client_fds[LISTENQ];
    int client_count;
    int flag;
    sem_t sem;
    pthread_mutex_t mutex;
}CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init(){
    CLIENT_REGISTRY *reg=Malloc(sizeof(CLIENT_REGISTRY));
    reg->client_count=0;
    reg->flag =0;
    for(int i=0;i<LISTENQ;i++){
        reg->client_fds[i] = -1;
    }
    Sem_init(&(reg->sem),0,0);
    pthread_mutex_init(&reg->mutex,NULL);
    pthread_mutex_init(&temp_mutex,NULL);
    return reg;
}

void creg_fini(CLIENT_REGISTRY *cr){
    pthread_mutex_lock(&temp_mutex);
    debug("in creg fini");
    Free(cr);
    pthread_mutex_unlock(&temp_mutex);
}

void creg_register(CLIENT_REGISTRY *cr, int fd){
    pthread_mutex_lock(&(cr->mutex));
    for(int i=0;i<LISTENQ;i++){
        if(cr->client_fds[i]<0){
            cr->client_fds[i]=fd;
            break;
        }
        if(i== LISTENQ){
            pthread_mutex_unlock(&(cr->mutex));
            unix_error("Too many clients");
        }
    }
    cr->client_count++;
    debug("Registered client  :%d and total count is:%d",fd, cr->client_count);
    pthread_mutex_unlock(&(cr->mutex));
}

void creg_unregister(CLIENT_REGISTRY *cr, int fd){
    pthread_mutex_lock(&(cr->mutex));
    debug("creg_unregister");
    if(fd < 0){
        pthread_mutex_unlock(&(cr->mutex));
        unix_error("client not registered");
    }
    for(int i=0;i<LISTENQ;i++){
        if(cr->client_fds[i]==fd){
            cr->client_fds[i]=-1;
            cr->client_count--;
            debug("Unregistered client  :%d and total count is:%d",fd, cr->client_count);
            if(cr->client_count == 0){
                debug("flag set to 1");
                cr->flag =1;
                V(&(cr->sem));
                debug("client count is 0");
            }
            break;
        }
        if(i== LISTENQ){
            pthread_mutex_unlock(&(cr->mutex));
            unix_error("client not registered");
        }
    }
    pthread_mutex_unlock(&(cr->mutex));
}

void creg_wait_for_empty(CLIENT_REGISTRY *cr){
    pthread_mutex_lock(&(cr->mutex));
    if(cr->flag==0){
        debug("waiting for zero client count is:%d",cr->client_count);
        pthread_mutex_unlock(&(cr->mutex));
        P(&(cr->sem));
        pthread_mutex_lock(&(cr->mutex));
    }
    pthread_mutex_unlock(&(cr->mutex));
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    debug("shutting down all");
    pthread_mutex_lock(&(cr->mutex));
    for(int i=0;i<LISTENQ;i++){
        if(cr->client_fds[i]>0){
            shutdown(cr->client_fds[i],SHUT_RD);
            cr->client_fds[i]=-1;
            cr->client_count--;
        }
    }
    pthread_mutex_unlock(&(cr->mutex));
}