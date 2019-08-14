#include <string.h>
#include "data.h"
#include "store.h"
#include "csapp.h"
#include "debug.h"
#include <math.h>

BLOB *blob_create(char *content, size_t size){
    debug("creating blob");
    BLOB* new_blob = Malloc(sizeof(BLOB));
    new_blob->size  = size;
    new_blob->refcnt = 0;
    new_blob->prefix = Malloc(size+1);
    memcpy(new_blob->prefix,content, size);
    new_blob->prefix[size] = '\0';
    new_blob->content = Malloc(size);
    memcpy(new_blob->content, content, size);
    pthread_mutex_init(&new_blob->mutex,  NULL);
    blob_ref(new_blob,"Increase refcnt for newly created job");
    return new_blob;
}
BLOB *blob_ref(BLOB *bp, char *why){
    pthread_mutex_lock(&(bp->mutex));
    debug("increasing blob ref cnt for%s",why);
    bp->refcnt++;
    pthread_mutex_unlock(&(bp->mutex));
    return bp;
}

void blob_unref(BLOB *bp, char *why){
    debug("bp is:%p",bp);
    pthread_mutex_lock(&(bp->mutex));
    bp->refcnt--;
    if(bp->refcnt==0){
        debug("count s zero free blob");
        Free(bp->prefix);
        Free(bp->content);
        Free(bp);
        return;
    }
    pthread_mutex_unlock(&bp->mutex);
}

int blob_compare(BLOB *bp1, BLOB *bp2){
    if(bp1->size == bp2->size){
        return memcmp(bp1->content, bp2->content, bp1->size);
    }
    return -1;
}
int blob_hash(BLOB *bp){
    int hash = strlen(bp->prefix)% 8;
    debug("hash is:%d",hash);
    return hash;
}
KEY *key_create(BLOB *bp){
    KEY *new_key = Malloc(sizeof(KEY));
    new_key->blob = bp;
    new_key->hash = blob_hash(bp);
    debug("key created is :%p",new_key);
    debug("key content is:%p",new_key->blob->content);
    return new_key;
}

void key_dispose(KEY *kp){
    if(kp!=NULL){
        debug("Disposing key");
        blob_unref(kp->blob,"Disposing Key");
        Free(kp);
    }
}

int key_compare(KEY *kp1, KEY *kp2){
    if(kp1->hash == kp2->hash){
        return blob_compare(kp1->blob,kp2->blob);
    }
    else{
        return -1;
    }
}

VERSION *version_create(TRANSACTION *tp, BLOB *bp){
    debug("blob is :%p",bp);
    pthread_mutex_lock(&(tp->mutex));
    VERSION *new_version = Malloc(sizeof(VERSION));
    new_version->blob = bp;
    new_version->creator = tp;
    new_version->next = NULL;
    new_version->prev = NULL;
    debug("before trans_ref refcnt s:%d",new_version->creator->refcnt);
    pthread_mutex_unlock(&(tp->mutex));
    trans_ref(new_version->creator,"Creating a new version");
    return new_version;
}
void version_dispose(VERSION *vp){

    trans_unref(vp->creator,"Disposing the given version");
    if(vp->blob!=NULL)
        blob_unref(vp->blob,"Disposing version");
    Free(vp);
}
