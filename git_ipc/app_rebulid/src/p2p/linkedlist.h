/*============================================================ * Author:	Wang tsmyfau@gmail.com * Filename:	linkedlist.h
 * Describle:
 * History: 
 * Last modified: 2014-07-17 15:47
=============================================================*/

#ifndef JAP2P_LINKEDLIST_H
#define JAP2P_LINKEDLIST_H

#include <pthread.h>
#include "common.h"



// #ifdef __cplusplus
// extern "C"{
// #endif



typedef struct value {
	void *data;
	UINT32 sz;
}LListData;

typedef struct llist_node{
	LListData key;
	LListData value;
	struct llist_node* next;
}LListNode;

// typedef bool (*fLListMaker)(LListData *, const void *);//make key or value
// typedef bool (*fLListClear)(LListNode *pNode);
// typedef bool (*fLListEqual)(LListData srckey, LListData dstkey);
typedef void (*fLListPrint)(LListNode *hdrNode);


typedef struct llist_hdr{
	//fLListMaker key_maker; //for users make node->key
	//fLListMaker value_maker;//for users make node->value
	//fLListClear clear; //free the users self free(mem in key, value);
	//fLListEqual equal;//the way to know node's equal
	LListNode *tailNode;
	pthread_mutex_t mutex;
}LListHdr;

//init linkedlist 
LListNode *llist_init(void);
//clear all nodes hanged on the linkedlist hdr
bool llist_clear(LListNode *hdrNode);
//destroy the linkedlist
bool llist_destroy(LListNode **hdrNode);
void llist_print(LListNode *hdrNode, fLListPrint print);


//these interface wraped on the linkedlist_xx_base, when key is string,
bool string_llist_add(LListNode *hdrNode, const void *key, const void *value);
bool string_llist_del(LListNode *hdrNode, const void *key);
bool string_llist_update(LListNode *hdrNode,const void *key,const void *value);
CHAR *string_llist_get(LListNode *hdrNode, const void *key);
void string_llist_print(LListNode *hdrNode);


//these interface wraped on the linkedlist_xx_base, when key is int,
bool b8_llist_add(LListNode *hdrNode, const void *key, UINT32 key_sz, 
									const void *value, UINT32 value_sz);
bool b8_llist_del(LListNode *hdrNode, const void *key, UINT32 key_sz);
bool b8_llist_update(LListNode *hdrNode, const void *key, UINT32 key_sz, 
										const void *value, UINT32 value_sz);
void *b8_llist_get(LListNode *hdrNode, const void *key, UINT32 key_sz);
//get key from value, notice , sz may not the value actual size
void *b8_llist_get2(LListNode *hdrNode, const void *value, UINT32 value_sz);


// #ifdef __cplusplus
// }
// #endif

#endif //end of linkedlist.h
