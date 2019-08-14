#include "transaction.h"
#include "store.h"
void add_transaction(TRANSACTION trans_list);

void traverse_trans_list(TRANSACTION* tp);
int traverse_depends_list(TRANSACTION* tp, TRANSACTION *dtp);

int check_dependency(TRANSACTION* dtp, DEPENDENCY *depends);
void check_depends(TRANSACTION* new_trans,int trans_id);


//traverse store and check for dependency
void traverse_store(TRANSACTION* head);

//traverse the map entry and check for dependency of head
void traverse_mapentry(MAP_ENTRY* map_entry, TRANSACTION* head);


//traverse the version and add dependency
void traverse_versions(VERSION* versions, TRANSACTION* head);


//check if head is present in versions provided
int check_head_in_versions(VERSION* version_head, TRANSACTION* head);

//remove the specified transaction from translist
void remove_from_translist(TRANSACTION* tp);

//delete the dependency list
void delete_dependency_list(DEPENDENCY* head, TRANSACTION* trans);


//deleting the trans_list
void delete_translist(TRANSACTION* head);