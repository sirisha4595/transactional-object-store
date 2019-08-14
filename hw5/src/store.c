// #include "store.h"
// #include "transaction.h"
// #include "data.h"
// #include "my_store.h"
// #include "data.h"
// #include "debug.h"
// #include "csapp.h"

// void store_init(void){
//     the_map.table = (MAP_ENTRY**)Malloc(NUM_BUCKETS * sizeof(MAP_ENTRY**));
//     the_map.num_buckets = NUM_BUCKETS;
//     pthread_mutex_init(&the_map.mutex,  NULL);
//     pthread_mutex_lock(&(the_map.mutex));
//     for(int i=0;i<the_map.num_buckets;i++){
//         the_map.table[i] = (MAP_ENTRY*)Malloc(sizeof(MAP_ENTRY));
//         the_map.table[i]->key = NULL;
//         //the_map.table[i]->key->blob = NULL;
//         the_map.table[i]->versions = NULL;
//         the_map.table[i]->next = NULL;
//     }
//     pthread_mutex_unlock(&(the_map.mutex));

// }

// void store_fini(void){
//     pthread_mutex_lock(&(the_map.mutex));
//     for(int i=0;i<NUM_BUCKETS;i++){
//         key_dispose(the_map.table[i]->key);
//         if(the_map.table[i]->versions!=NULL){
//             free_list(the_map.table[i]->versions);
//         }
//         Free(the_map.table[i]);
//     }

// }

// TRANS_STATUS store_put(TRANSACTION *tp, KEY *key, BLOB *value){
//     //int hash = get_store_hash(key);
//     pthread_mutex_lock(&(the_map.mutex));
//     if(key!=NULL){
//         int hash = key->hash;
//         VERSION* versions=traverse_map(key, the_map.table[hash]);
//         if(versions != NULL)
//             garbage_collection(hash);
//         else
//             the_map.table[hash] = add_key(the_map.table[hash],key);
//         if(check_transid_greater(tp->id,hash)==0){
//             pthread_mutex_unlock(&(the_map.mutex));
//             return trans_abort(tp);
//         }
//         else{
//             int max_transid;
//             if(the_map.table[hash]!= NULL && the_map.table[hash]->versions!=NULL)
//                 max_transid= get_max_id(hash);
//             else
//                 max_transid = -1;
//             VERSION* version = version_create(tp,value);
//             // if(versions==NULL)
//             //     the_map.table[hash] = add_key(the_map.table[hash],key);
//             if(max_transid == -1){
//                 the_map.table[hash]->versions = add_version_end(version, the_map.table[hash]->versions);
//             }
//             else if(max_transid == tp->id){
//                 //replace_last_version(version, the_map.table[key->hash]->versions);
//                 replace_last_version(version, key);
//             }
//             else if(max_transid < tp->id){
//                 debug("Yes it is maxid");
//                 the_map.table[hash]->versions = add_version_end(version, the_map.table[hash]->versions);
//             }
//             add_earlier_dependency_list(version, hash);
//             //version->blob = value;
//             //version->creator->refcnt++;
//             //blob_ref(version->blob,"Added version");
//             // debug("tp is:%p",tp);
//             // //debug("tp->content is:%s",tp->content);
//             // debug("key  is :%p",key);
//             // debug("key->blob->key is:%s",key->blob->content);
//             // debug("value pointer is:%p",value);
//             // debug("value content is:%s",value->content);
//         }
//     }

//     pthread_mutex_unlock(&(the_map.mutex));
//     //store_show();
//     return trans_get_status(tp);
// }

// TRANS_STATUS store_get(TRANSACTION *tp, KEY *key, BLOB **valuep){
//     pthread_mutex_lock(&(the_map.mutex));
//     if(key!=NULL){
//         int hash = key->hash;
//         VERSION* versions=traverse_map(key, the_map.table[hash]);
//         if(versions!= NULL)
//             garbage_collection(hash);
//         //else
//         //    the_map.table[hash] = add_key(the_map.table[hash],key);
//         if(check_transid_greater(tp->id,hash)==0){
//             pthread_mutex_unlock(&(the_map.mutex));
//             return trans_abort(tp);
//         }
//         else{
//             int max_transid;
//             if(the_map.table[hash]->versions!=NULL)
//                 max_transid= get_max_id(hash);
//             else
//                 max_transid = -1;
//             BLOB* blob = get_preceding_value(hash,tp);
//             if(blob!=NULL)
//                 blob_ref(blob," For creating new version");
//             VERSION* version = version_create(tp,blob);
//             if(versions == NULL)
//                 the_map.table[hash] = add_key(the_map.table[hash],key);
//             debug("version->blbo is:%p",version->blob);
//             if(max_transid == -1){
//                 the_map.table[hash]->versions = add_version_end(version, the_map.table[hash]->versions);
//             }
//             else if(max_transid == tp->id){

//                 //replace_last_version(version, the_map.table[key->hash]->versions);
//                 replace_last_version(version, key);
//             }
//             else if(max_transid < tp->id){
//                 debug("Yes it is maxid");
//                 add_version_end(version, the_map.table[hash]->versions);
//             }
//             add_earlier_dependency_list(version, hash);
//             *valuep = version->blob;
//             if(blob!=NULL)
//                 blob_ref(blob," for returning from store_get");
//         }
//     }
//     pthread_mutex_unlock(&(the_map.mutex));
//     return trans_get_status(tp);
// }

// void store_show(void){
//     fprintf(stderr, "%s\n","CONTENT OF THE STORE");
//     for(int i=0;i<the_map.num_buckets;i++){
//         // if(the_map.table[i] == NULL){
//         //     fprintf(stderr, "%d:\n",i);
//         // }
//         // else{
//         //fprintf(stderr,"%d:\t",i);
//         MAP_ENTRY* head1 = the_map.table[i];
//         if(head1->key ==  NULL){
//             fprintf(stderr, "\n%d:",i);
//             continue;
//         }
//         else{
//             fprintf(stderr, "\n%d:\t",i);
//             while(head1!=NULL && head1->key!=NULL){

//                 fprintf(stderr, "\n\t{key: %p[%s],",head1->key,head1->key->blob->prefix);
//                 VERSION* head2 = head1->versions;
//                 while(head2!=NULL){
//                     fprintf(stderr, "versions: {creator = %d",head2->creator->id);
//                     if(head2->creator->status ==  TRANS_ABORTED)
//                         fprintf(stderr,"(aborted),");
//                     else if(head2->creator->status ==  TRANS_COMMITTED)
//                         fprintf(stderr,"(committed),");
//                     else if(head2->creator->status == TRANS_PENDING)
//                         fprintf(stderr,"(pending),");
//                     if(head2->blob!=NULL)
//                         fprintf(stderr,"blob = %p [%s]},",head2->blob,head2->blob->prefix);
//                     else
//                         fprintf(stderr,"blob = null [(null)]}}");
//                     head2 = head2->next;
//                 }
//                 head1 = head1->next;
//             }
//         }
//     }
// }

// void free_list(VERSION* versions){
//     VERSION* temp;
//     while(versions!=NULL){
//         temp = versions;
//         versions = versions->next;
//         version_dispose(temp);
//     }
// }

// // int get_store_hash(KEY* key){
// //     int i;
// //     for(i=0;i<NUM_BUCKETS;i++){
// //         if(store[i].key->hash == key->hash)
// //             break;
// //     }
// //     return i;
// // }

// void garbage_collection(int hash){
//     if(the_map.table[hash]->versions!=NULL){
//         VERSION* head = the_map.table[hash]->versions;
//         VERSION* most_recent_commit = NULL;
//         while(head!=NULL){
//             if(head->creator->status == TRANS_COMMITTED){
//                 if(most_recent_commit!=NULL){
//                     remove_node(hash,most_recent_commit);
//                 }
//                 most_recent_commit = head;
//             }
//             // else if(head->creator->status == TRANS_PENDING){
//             //     remove_node(hash,head);
//             // }
//             else if(head->creator->status == TRANS_ABORTED){
//                 remove_later_list(head);
//             }
//             head = head->next;
//         }
//     }
// }

// void remove_node(int hash, VERSION* most_recent_commit){
//     // Store head node
//     VERSION* temp = the_map.table[hash]->versions;
//     VERSION* prev;
//     // If head node itself holds the key to be deleted
//     if ( the_map.table[hash]->versions != NULL && the_map.table[hash]->versions == most_recent_commit) {
//         the_map.table[hash]->versions = the_map.table[hash]->versions->next;   // Changed head
//         //free(temp);
//         debug("temp is:%p",temp);       // free old head
//         version_dispose(temp);
//         return;
//     }
//     // Search for the key to be deleted, keep track of the
//     // previous node as we need to change 'prev->next'
//     while (the_map.table[hash]->versions != NULL && the_map.table[hash]->versions != most_recent_commit){
//         prev = the_map.table[hash]->versions;
//         the_map.table[hash]->versions = the_map.table[hash]->versions->next;
//     }
//     // If key was not present in linked list
//     if (the_map.table[hash]->versions == NULL) {
//         the_map.table[hash]->versions = temp;
//         return;
//     }
//     // Unlink the node from linked list
//     prev->next = the_map.table[hash]->versions->next;
//     the_map.table[hash]->versions->next->prev = prev;
//     //Free(temp);  // Free memory
//     version_dispose(the_map.table[hash]->versions);
//     the_map.table[hash]->versions = temp;

// }
// void remove_later_list(VERSION* head){
//     VERSION* curr =head;
//     VERSION* next;
//     while(curr!=NULL){
//         next = curr->next;
//         //curr->creator->status =  TRANS_ABORTED;
//         //TRANSACTION* tp = curr->creator;
//         version_dispose(curr);
//         //trans_abort(tp);
//         curr = next;
//     }
//     head= NULL;
// }

// int check_transid_greater(int id,int hash){
//     VERSION* head = the_map.table[hash]->versions;
//     int max_id;
//     if(head!=NULL){
//         max_id = get_max_id(hash);
//         if(id>=max_id)
//             return 1;
//         else
//             return 0;
//     }
//     return 1;

//     // if(the_map.table[hash]->versions == NULL)
//     //     return 1;
//     // int max_id = -1;
//     // while(the_map.table[hash]->versions != NULL){
//     //     if(id == the_map.table[hash]->versions->creator->id){
//     //         the_map.table[hash]->versions = head;
//     //         return 1;
//     //     }
//     //     if(max_id < the_map.table[hash]->versions->creator->id)
//     //         max_id = the_map.table[hash]->versions->creator->id;
//     //     the_map.table[hash]->versions = the_map.table[hash]->versions->next;
//     // }
//     // if( id >  max_id){
//     //     the_map.table[hash]->versions = head;
//     //     return 1;
//     // }
//     // the_map.table[hash]->versions = head;
//     return 0;
// }

// int get_max_id(int hash){
//     VERSION* head = the_map.table[hash]->versions;
//     int max = the_map.table[hash]->versions->creator->id;
//     the_map.table[hash]->versions = the_map.table[hash]->versions->next;
//     while(the_map.table[hash]->versions != NULL){
//         if(max <  the_map.table[hash]->versions->creator->id){
//             max = the_map.table[hash]->versions->creator->id;
//         }
//         the_map.table[hash]->versions = the_map.table[hash]->versions->next;
//     }
//     the_map.table[hash]->versions = head;
//     return max;
// }
// void replace_last_version(VERSION* version, KEY* key){


//     //VERSION* head1 = versions;
//     int hash = key->hash;
//     VERSION* head = the_map.table[hash]->versions;
//     key_dispose(key);
//     if(the_map.table[hash]->versions->next ==NULL){
//         VERSION* temp = the_map.table[hash]->versions;
//         the_map.table[hash]->versions = version;
//         version_dispose(temp);
//         //the_map.table[hash]->versions = head;
//         return;
//     }
//     while(the_map.table[hash]->versions->next!=NULL){
//         debug("the_map.table[hash]->versions is:%p",the_map.table[hash]->versions);
//         the_map.table[hash]->versions = the_map.table[hash]->versions->next;
//     }
//     VERSION* temp = the_map.table[hash]->versions;
//     the_map.table[hash]->versions->prev->next = version;
//     version->prev = the_map.table[hash]->versions->prev;
//     //the_map.table[hash]->versions = version;

//     version_dispose(temp);
//     the_map.table[hash]->versions =head;

// }

// VERSION* add_version_end(VERSION* version, VERSION* versions){
//     //VERSION *last = versions;
//     //version->next = NULL;
//     if (versions == NULL){
//        versions = version;
//        versions->next = NULL;
//        versions->prev = NULL;
//        return versions;
//     }
//     else if(versions->next == NULL){
//         versions->next = version;
//         version->prev = versions;
//         return versions;
//     }
//     VERSION* head =  versions;
//     while (head->next != NULL)
//         head = head->next;
//     head->next = version;
//     version->prev =  head;
//     return versions;

// }

// void add_earlier_dependency_list(VERSION* version, int hash){
//     VERSION* head = the_map.table[hash]->versions;
//     while(head!=NULL){
//         if(head->creator != version->creator && head->creator->status == TRANS_PENDING){
//             trans_add_dependency(version->creator, head->creator);
//         }
//         head = head->next;
//     }
//     //the_map.table[hash]->versions = head;
// }

// VERSION* traverse_map(KEY* key, MAP_ENTRY* map_entry){
//     MAP_ENTRY* head = the_map.table[key->hash];
//     while(head!=NULL){
//         if(head->key!=NULL && key_compare(head->key, key) == 0){
//             return head->versions;
//         }
//         head = head->next;
//     }
//     return NULL;
// }

// MAP_ENTRY* add_key(MAP_ENTRY* row,KEY* key){
//     // MAP_ENTRY* new = Malloc(sizeof(MAP_ENTRY));
//     // new->key = key;
//     // new->versions = NULL;
//     // new->next = row;
//     // row = new;
//     MAP_ENTRY* new_node = Malloc(sizeof(MAP_ENTRY));

//     /* 2. put in the data  */
//     new_node->key  = key;
//     new_node->versions = NULL;
//     /* 3. Make next of new node as head */
//     new_node->next = row;

//     /* 4. move the head to point to the new node */
//     (row)    = new_node;
//     return row;
// }

// BLOB* get_preceding_value(int hash, TRANSACTION* tp){
//     VERSION* head = the_map.table[hash]->versions;
//     if(the_map.table[hash]->versions ==  NULL){
//         return NULL;
//     }
//     if(the_map.table[hash]->versions->next ==NULL){
//         //the_map.table[hash]->versions = head;
//         return the_map.table[hash]->versions->blob;
//     }
//     while(the_map.table[hash]->versions->next!=NULL){
//         debug("the_map.table[hash]->versions is:%p",the_map.table[hash]->versions);
//         the_map.table[hash]->versions = the_map.table[hash]->versions->next;
//     }
//     BLOB* blob = the_map.table[hash]->versions->blob;
//     the_map.table[hash]->versions =head;
//     return blob;
// }