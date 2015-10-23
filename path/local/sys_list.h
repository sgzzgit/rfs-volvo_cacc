/**\file	
 *	sys_list.h
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *	Structures for lists, as used by sys_list.c
 *	Last linked list node has pnext == NULL.
 *	First double linked list node has pfirst == NULL.
 *
 */
#ifndef PATH_SYS_LIST_H
#define PATH_SYS_LIST_H
#include <local.h>

typedef struct ll_node_str 
{
	void *pitem;
	struct ll_node_str *pnext;
} ll_node_typ;

typedef struct
{
	int length;
	ll_node_typ *pfirst, *plast;
} ll_head_typ;

typedef struct dl_node_str
{
	void *pitem;
	struct dl_node_str *pprev, *pnext;
} dl_node_typ;

typedef struct
{
	int length;
	dl_node_typ *pfirst, *plast;
} dl_head_typ;

extern ll_head_typ *ll_create( void );
extern void ll_free(ll_head_typ *phead);
extern ll_node_typ *ll_first( ll_head_typ *phead, void *pkey, int (*pfkeycmp)());
extern ll_node_typ *ll_next( ll_node_typ *pnode, void *pkey, int (*pfkeycmp)());
extern int ll_insert( void *pitem, ll_head_typ *phead );
extern int ll_append( void *pitem, ll_head_typ *phead );
extern void *ll_rm_first( ll_head_typ *phead );
extern void *ll_rm_last( ll_head_typ *phead );
extern void *ll_rm_node( ll_head_typ *phead, ll_node_typ **pcurrent, 
		ll_node_typ *pold );
extern int ll_length( ll_head_typ *phead );
extern dl_head_typ *dl_create( void );
extern void dl_free( dl_head_typ *phead );
extern int dl_insert( void *pitem, dl_head_typ *phead );
extern int dl_append( void *pitem, dl_head_typ *phead );
extern void *dl_rm_first( dl_head_typ *phead );
extern void *dl_rm_last( dl_head_typ *phead );
extern void *dl_rm_node( dl_head_typ *phead, dl_node_typ **pcurrent );
extern int dl_mv_node( void *pitem, 
		dl_head_typ *phead, unsigned size, int maximum );
dl_node_typ *dl_nth_node( int n, dl_head_typ *phead );
int dl_nth_insert( int n, void *pitem, dl_head_typ *phead );

extern dl_node_typ *dl_first( dl_head_typ *phead, void *pkey, int (*pfkeycmp)());
extern dl_node_typ *dl_next( dl_node_typ *pref_node, void *pkey, 
							int (*pfkeycmp)());
extern bool_typ dl_pq_add( dl_head_typ *phead, void *pitem, 
							int (*pfcmp)(), unsigned size );
extern int dl_length( dl_head_typ *phead );
extern bool_typ dl_add_dup( dl_head_typ *phead, void *pdata, 
			unsigned size, bool_typ front );
#endif
