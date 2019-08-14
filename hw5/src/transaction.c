// #include "transaction.h"
// #include "my_transaction.h"
// #include "store.h"
// #include "csapp.h"
// #include "debug.h"

// TRANSACTION* sentinel =NULL;

// void trans_init(void){
//     debug("trans_init");
//     sentinel = Malloc(sizeof(TRANSACTION));
//     sentinel->id = -1;
//     sentinel->refcnt = -1;
//     sentinel->status = -1;
//     sentinel->depends = NULL;
//     sentinel->waitcnt =-1;
//     Sem_init(&(sentinel->sem),0,0);
//     pthread_mutex_init(&sentinel->mutex,  NULL);
//     trans_list = *sentinel;
//     trans_list.next = &trans_list;
//     trans_list.prev = &trans_list;
// }


// void trans_fini(void){
//     debug("trans_fini");
//     pthread_mutex_lock(&trans_list.mutex);
//     debug("trans_finfi");
//     delete_translist(&trans_list);
// }


// TRANSACTION *trans_create(void){
//     debug("trans_create");
//     TRANSACTION *new_transaction = Malloc(sizeof(TRANSACTION));
//     if(new_transaction == NULL)
//         return NULL;
//     else{
//         if(trans_list.next == &trans_list && trans_list.prev ==  &trans_list){//first transaction
//             new_transaction->id = 0;           // Transaction ID.
//             new_transaction->refcnt = 1;       // Number of references (pointers) to transaction.
//             new_transaction->status = TRANS_PENDING;       // Current transaction status.
//             new_transaction->depends = NULL;       // Singly-linked list of dependencies.
//             new_transaction->waitcnt = 0;
//             Sem_init(&(new_transaction->sem),0,0);
//             pthread_mutex_init(&new_transaction->mutex,  NULL);               // Number of transactions waiting for this one.
//             new_transaction ->next = &trans_list;
//             new_transaction->prev = &trans_list;
//             trans_list.next = new_transaction;
//             trans_list.prev = new_transaction;
//         }
//         else{//not first transaction
//             new_transaction->id = trans_list.next->id + 1;           // Transaction ID.
//             new_transaction->refcnt = 1;       // Number of references (pointers) to transaction.
//             new_transaction->status = TRANS_PENDING;       // Current transaction status.
//             new_transaction->depends = NULL;       // Singly-linked list of dependencies.
//             //
//             Sem_init(&(new_transaction->sem),0,0);
//             pthread_mutex_init(&new_transaction->mutex,NULL);
//             //check_depends(new_transaction,new_transaction->id);
//             new_transaction->waitcnt = 0;
//                           // Number of transactions waiting for this one.
//             new_transaction ->next = trans_list.next;
//             new_transaction->next->prev = new_transaction;
//             new_transaction->prev = &trans_list;
//             trans_list.next = new_transaction;
//         }
//     }
//     return new_transaction;
// }

// TRANSACTION *trans_ref(TRANSACTION *tp, char *why){
//     pthread_mutex_lock(&(tp->mutex));
//     tp->refcnt++;
//     debug("trans_ref increase from :%d->%d",tp->refcnt-1,tp->refcnt);
//     pthread_mutex_unlock(&(tp->mutex));
//     return tp;
// }

// void trans_unref(TRANSACTION *tp, char *why){
//     debug("trans_unref");
//     if(tp){
//         debug("tp not NULL");
//         pthread_mutex_lock(&(tp->mutex));
//         //remove_from_translist(tp);
//         if(tp->refcnt<0)
//             debug("Refcnt < 0");
//         tp->refcnt--;
//         debug("trans_ref decrease from :%d->%d",tp->refcnt+1,tp->refcnt);
//         if(tp->refcnt==0){
//             //pthread_mutex_unlock(&(tp->mutex));
//             remove_from_translist(tp);
//             //pthread_mutex_lock(&(tp->mutex));
//             return;
//         }
//         //trans_show_all();
//         debug("unlocked");
//         pthread_mutex_unlock(&(tp->mutex));
//     }
// }


// void trans_add_dependency(TRANSACTION *tp, TRANSACTION *dtp){
//     debug("trans_add_dependency");
//     pthread_mutex_lock(&(tp->mutex));
//     if(tp->depends == NULL){//first dependency
//         tp->depends = Malloc(sizeof(DEPENDENCY));
//         tp->depends->trans = dtp;
//         tp->depends->next =NULL;
//         trans_ref(dtp, " for transaction in dependency");
//         // pthread_mutex_lock(&(dtp->mutex));
//         // dtp->refcnt++;
//         // pthread_mutex_unlock(&(dtp->mutex));
//     }
//     else{//insert at head
//         if(check_dependency(dtp, tp->depends)==0){//check if this dependency is already in the list
//             DEPENDENCY *new_dependency = Malloc(sizeof(DEPENDENCY));
//             new_dependency->trans = dtp;
//             new_dependency->next = tp->depends;
//             tp->depends = new_dependency;
//             trans_ref(dtp, " for transaction in dependency");
//             // pthread_mutex_lock(&(dtp->mutex));
//             // dtp->refcnt++;
//             // pthread_mutex_unlock(&(dtp->mutex));
//         }
//     }
//     pthread_mutex_unlock(&(tp->mutex));
// }

// TRANS_STATUS trans_commit(TRANSACTION *tp){
//     debug("trans_commit");
//     pthread_mutex_lock(&(tp->mutex));
//     DEPENDENCY *head = tp->depends;
//     while(head!=NULL){
//         debug("head not NULL");
//         debug("head unlocked");

//         if(head->trans->status == TRANS_PENDING){
//             head->trans->waitcnt++;
//             debug("P opeation is pending");
//             pthread_mutex_unlock(&(tp->mutex));
//             P(&(head->trans->sem));
//             debug("done with V");
//             pthread_mutex_lock(&(tp->mutex));
//         }
//         if(head->trans->status == TRANS_ABORTED){
//             debug("inside trans_aboretd");
//             traverse_trans_list(tp);
//             pthread_mutex_unlock(&(tp->mutex));
//             trans_unref(tp, "Aborting the transaction");
//             pthread_mutex_lock(&(tp->mutex));
//             tp->status = TRANS_ABORTED;
//             return TRANS_ABORTED;
//         }
//         if(head->trans->status == TRANS_COMMITTED){
//             head = head->next;
//         }
//     }
//     debug("after while");
//     while(tp->waitcnt > 0){
//         //pthread_mutex_unlock(&(tp->mutex));
//         debug("V operation in committed");
//         tp->waitcnt--;
//         V(&tp->sem);
//         // pthread_mutex_unlock(&(tp->mutex));
//         // //trans_unref(tp, "Committing the transaction");
//         // pthread_mutex_lock(&(tp->mutex));
//     }
//     tp->status = TRANS_COMMITTED;
//     trans_unref(tp, "Committing the transaction");
//     store_show();
//     trans_show_all();
//     pthread_mutex_unlock(&(tp->mutex));
//     return TRANS_COMMITTED;
// }

// TRANS_STATUS trans_abort(TRANSACTION *tp){
//     debug("trans_abort");
//     pthread_mutex_lock(&(tp->mutex));
//     debug("locked");
//     //trans_unref(tp,"");

//     if(tp->status != TRANS_COMMITTED){
//         if(tp->status == TRANS_PENDING){
//             tp->status = TRANS_ABORTED;
//             traverse_trans_list(tp);
//         }
//         tp->status = TRANS_ABORTED;
//         pthread_mutex_unlock(&(tp->mutex));
//         trans_unref(tp, "for aborting the transaction");
//         return TRANS_ABORTED;
//     }
//     pthread_mutex_unlock(&(tp->mutex));
//     trans_unref(tp, "for commnting the transaction");
//     return TRANS_COMMITTED;
// }

// TRANS_STATUS trans_get_status(TRANSACTION *tp){
//     debug("trans_status");
//     pthread_mutex_lock(&(tp->mutex));
//     int status = tp->status;
//     pthread_mutex_unlock(&(tp->mutex));
//     return status;
// }

// void trans_show(TRANSACTION *tp){
//     debug("trans_show");
//     fprintf(stderr,"TRANSACTIONS:\n");
//     fprintf(stderr,"[id:%d, status:%d, refcnt:%d]",tp->id,tp->status,tp->refcnt);
// }

// void trans_show_all(void){
//     debug("trans_show_all");
//     TRANSACTION* head = &trans_list;
//     head=head->next;
//     fprintf(stderr,"TRANSACTIONS:\n");
//     while(head!= &trans_list){
//         fprintf(stderr,"[id:%d, status:%d, refcnt:%d]\n",head->id,head->status,head->refcnt);
//         head = head->next;
//     }
// }

// void traverse_trans_list(TRANSACTION* tp){
//     debug("locked");
//     TRANSACTION *head= &trans_list;
//     head=(head->next);
//     while(head!=&trans_list){
//         debug("locked head");
//         TRANSACTION* temp = head->next;
//         if(head->depends!=NULL){
//             debug("unlocked tp");
//             traverse_depends_list(tp,head);
//         }
//         head = temp;
//     }
// }
// int traverse_depends_list(TRANSACTION *tp,TRANSACTION *dtp){
//     DEPENDENCY* head= dtp->depends;
//     while(head!=NULL){
//         debug("head->trans :%p",head->trans);
//         debug("tp is :%p",tp);
//         if(head->trans== tp){
//             while(head->trans->waitcnt > 0){
//                 head->trans->status = TRANS_ABORTED;
//                 head->trans->waitcnt--;

//                 V(&head->trans->sem);
//                 debug("V done");
//             }
//             return 1;
//         }
//         head = head->next;
//     }
//     return 0;
// }

// int check_dependency(TRANSACTION* dtp, DEPENDENCY *depends){
//     DEPENDENCY* head = depends;
//     while(head){
//         if(head->trans == dtp)
//             return 1;
//         head=head->next;
//     }
//     return 0;
// }

// void check_depends(TRANSACTION* new_trans,int trans_id){
//     TRANSACTION *head= &trans_list;
//     head=(head->next);
//     while(head!=&trans_list){
//         if(head->id < trans_id && head->status == TRANS_PENDING && head->refcnt>1){//made entry to the store
//         }
//         head = head->next;
//     }
// }

// void traverse_store(TRANSACTION* head){
//     for(int i=0;i < NUM_BUCKETS;i++){
//         traverse_mapentry(the_map.table[i], head);
//     }
// }

// void traverse_mapentry(MAP_ENTRY* map_entry, TRANSACTION* head){
//     MAP_ENTRY* map_entry_head = map_entry;
//     while(map_entry_head!=NULL){
//         if(check_head_in_versions(map_entry_head->versions, head)==1){
//             traverse_versions(map_entry_head->versions, head);
//         }
//         map_entry_head = map_entry_head->next;
//     }
// }

// void traverse_versions(VERSION* versions, TRANSACTION* head){
//     VERSION* version_head = versions;
//     while(version_head!=NULL){
//         if(version_head->creator->id < head->id && version_head->creator->status == TRANS_PENDING)
//             trans_add_dependency(head,version_head->creator);
//         version_head = version_head->next;
//     }
// }

// int check_head_in_versions(VERSION* version, TRANSACTION* head){
//     VERSION* version_head = version;
//     while(version_head!=NULL){
//         if(version_head->creator->id == head->id){
//             return 1;
//         }
//         version_head = version_head->next;
//     }
//     return 0;
// }

// void remove_from_translist(TRANSACTION* tp){
//     //pthread_mutex_lock(&tp->mutex);
//     TRANSACTION* head = &trans_list;
//     head = head->next;
//     while(head!=tp){
//         head = head->next;
//     }
//     head->prev->next = head->next;
//     head->next->prev = head->prev;
//     if(head->depends!=NULL){
//         //pthread_mutex_unlock(&tp->mutex);
//         delete_dependency_list(head->depends, tp);
//         //pthread_mutex_lock(&tp->mutex);
//     }
//     Free(head);
// }

// void delete_dependency_list(DEPENDENCY* head, TRANSACTION* trans){
//     debug("deletedepedency list");
//     debug("delete_dependency_list");
//     DEPENDENCY* curr =head;
//     DEPENDENCY* next;
//     while(curr!=NULL){

//         next = curr->next;
//         Free(curr);
//         curr = next;
//     }
//     head= NULL;
// }

// void delete_translist(TRANSACTION* head){
//     debug("delete_translist");
//     TRANSACTION* curr =head->next;
//     TRANSACTION* next;
//     while(curr!=head){
//         debug("locked");
//         next = curr->next;
//         delete_dependency_list(curr->depends, curr);
//         Free(curr);
//         curr = next;
//     }
// }