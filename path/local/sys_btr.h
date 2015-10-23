/**\file	
 *	sys_btr.h
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *	Node definition and prototypes for binary search tree routines.
 *
 */
#ifndef PATH_SYS_BTR_H
#define PATH_SYS_BTR_H
typedef struct btr_node_str 
{
	void *pitem;
	struct btr_node_str *pleft;
	struct btr_node_str *pright;
	int balance;
} btr_node_typ;

typedef enum
{
	BTR_PREORDER, 
	BTR_INORDER, 
	BTR_LEAF,
	BTR_POSTORDER
} btr_visit_typ;

btr_node_typ *btr_inst_node( void *pitem );
void *btr_add( void *pitem, btr_node_typ **proot, int (*pfcmp)() );
void *btr_avl( void *pitem, btr_node_typ **proot, int (*pfcmp)() );
void *btr_find( void *pitem, btr_node_typ **proot, int (*pfcmp)() );
void btr_destroy( btr_node_typ **proot, void (*pfitem)() );
void *btr_rm( void *pitem, btr_node_typ **proot, int (*pfcmp)() );
void btr_walk( btr_node_typ *proot, void (*pfwalk)() );

#endif
