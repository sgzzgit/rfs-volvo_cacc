/**\file	
 *
 *	sys_btr.c
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *
 *	Routines for maintaining an AVL binary search tree in
 *	memory.  
 *
 *	All routines handle only the pointers to data, so that 
 *	maintenance of the data itself is the responsibility
 *	of the application code.
 *
 *	Search and traversal routines are recursive, 
 *	so stack usage is dependent on the amount of data in the tree.
 *
 */

#include <sys_os.h>
#include "local.h"
#include "sys_btr.h"

static void btr_depth_left( btr_node_typ *pnode,
	void (*pfnode)(), void (*pfitem)(), int level );
static int avl_insert( void *pitem, btr_node_typ *pstart,
	int (*pfcmp)(), btr_node_typ **pq, btr_node_typ **ps, 
	btr_node_typ **pt );
static int avl_balance( void *pitem, int (*pfcmp)(), btr_node_typ *pq,
	btr_node_typ **pr, btr_node_typ *ps, int *pfactor );
static void item_func( btr_node_typ *pnode, void (*pfitem)(),
	btr_visit_typ order, int level );
static void dump_node( btr_node_typ *pnode, void (*pfitem)(),
	btr_visit_typ order, int level );
static btr_node_typ *avl_single( int factor, btr_node_typ *ps, 
	btr_node_typ *pr );
static btr_node_typ *avl_double( int factor, btr_node_typ *ps, 
	btr_node_typ *pr );

#define AVL_RIGHT			1
#define AVL_LEFT			(-1)
#define AVL_BALANCED		0

#define AVL_DONE			0
#define AVL_SINGLE			1
#define AVL_DOUBLE			2

#define LINK(a,pnode)		(a == AVL_LEFT ? pnode->pleft : pnode->pright)

/**	SYN/OPSIS
 *	
 *	#include <sys_btr.h>
 *
 *	DESCRIPTION
 *
 *	btr_node_typ *btr_inst_node( void *pitem )
 *
 *	Instantiate a tree node, setting the item pointer to the given data,
 *	the links to NULL, and the AVL balance factor to zero.
 *	
 *	RETURN
 *		NULL		if memory cannot be allocated for the node.
 *		non-NULL	Pointer to the node.
 */

btr_node_typ *btr_inst_node( pitem )
void *pitem;
{
	register btr_node_typ *pnode;

	if( (pnode = (btr_node_typ *) malloc( sizeof( btr_node_typ ))) == NULL )
		return( NULL );
	
	pnode->pitem = pitem;
	pnode->pleft = NULL;
	pnode->pright = NULL;
	pnode->balance = 0;

	return( pnode );
}

/**	SYN/OPSIS
 *	
 *	#include <sys_btr.h>
 *
 *	DESCRIPTION
 *
 *	void *btr_add( void *pitem, btr_node_typ **pparent, int (*pfcmp)() )
 *	pitem	-	Pointer to item to be added to the tree.
 *	pparent	-	Pointer to pointer to root node of tree.
 *	pfcmp	-	Pointer to user supplied comparison function of tree items.
 *	
 *	Does not balance the tree.
 *	Add a pointer to the the given item into the binary tree, or return a
 *	pointer to matching existing item which is already in the tree.  If
 *	the node is empty at the insertion point, e.g. an empty tree root, the
 *	item will be added at that node location.
 *
 *	RETURN
 *		NULL		if parent or search item is invalid.
 *		non-NULL	Pointer to the item.
 *					This could be the new item, or an old item
 *					with a matching key.
 */

void *btr_add( pitem, pparent, pfcmp )
void *pitem;
register btr_node_typ **pparent;
int (*pfcmp)();
{
	register int result;

	/*	
	 *	Quit if bad arguments.	
	 */

	if( (pitem == NULL) || (pparent == NULL) )
		return( NULL );

	/*	
	 *	Create a new node if we have descended from a leaf.
	 */

	if( *pparent == NULL )
	{
		if( (*pparent = btr_inst_node( pitem )) == NULL )
			return( NULL );
		else
			return( pitem );	
	}

	/*	
	 *	The current node has no data, e.g. the root for an empty tree.	
	 */

	if( (*pparent)->pitem == NULL )
		return( (*pparent)->pitem = pitem );

	result = pfcmp( pitem, (*pparent)->pitem );

	/*
	 *	If the new item is less than the current node, 
	 *	then add to the left side of the tree.
	 *	If the item matches the tree node, return the matching item.
	 *	Otherwise add the item to the right of the tree.
	 */

	if( result < 0 )
		return( btr_add( pitem, &((*pparent)->pleft), pfcmp ) );
	else if( result == 0 )
		return( (*pparent)->pitem );
	else
		return( btr_add( pitem, &((*pparent)->pright), pfcmp ) );
}

/**	SYN/OPSIS
 *	
 *	#include <sys_btr.h>
 *
 *	DESCRIPTION
 *
 *	void *btr_avl( void *pitem, btr_node_typ **proot, int (*pfcmp)() )
 *	pitem	-	Pointer to item to be added to the tree.
 *	proot	-	Pointer to pointer to root node of tree.
 *	pfcmp	-	Pointer to user supplied comparison function of tree items.
 *	
 *	Add a node to an AVL balanced tree, or return a pointer to matching
 *	existing item which is already in the tree.  If the node is empty at
 *	the insertion point, e.g. an empty tree root, the item will be added at
 *	that node location.
 *
 *	RETURN
 *		NULL		if parent or search item is invalid.
 *		non-NULL	Pointer to the item.
 *					This could be the new item, or an existing item
 *					with a matching key.
 */

void *btr_avl( pitem, proot, pfcmp )
void *pitem;
register btr_node_typ **proot;
int (*pfcmp)();
{
	register int result;
	int factor, rotation;
	btr_node_typ *pp, *pq, *pr, *ps, *pt;

	/*	
	 *	Quit if bad arguments.	
	 */

	if( (pitem == NULL) || (*proot == NULL) )
		return( NULL );

	/*	
	 *	The tree is empty, so set the node and quit.	
	 */

	if( (*proot)->pitem == NULL )
		return( (*proot)->pitem = pitem );

 	result = avl_insert( pitem, *proot, pfcmp, &pq, &ps, &pt );

 	if( result == FALSE )
		return( pq->pitem );
	else if( result == ERROR )
		return( NULL );
	else
		rotation = avl_balance( pitem, pfcmp, pq, &pr, ps, &factor );

	if( rotation == AVL_DONE )
		return( pitem );

	if( rotation == AVL_SINGLE )
		pp = avl_single( factor, ps, pr );
	else if( rotation == AVL_DOUBLE )
		pp = avl_double( factor, ps, pr );

	if( ps == *proot )
		*proot = pp;
	else
	{
		if( ps == pt->pright )
			pt->pright = pp;
		else
			pt->pleft = pp;
	}

	return( pitem );
}

/**	SYN/OPSIS
 *
 *	#include <sys_btr.h>
 *
 *	static int avl_insert( pitem, pstart, pfcmp, pq, ps, pt )
 *
 *	void *pitem			-	Pointer to the new data item.
 *	btr_node_typ *pstart	-	Pointer to subtree where insertion starts.
 *	int (*pfcmp)()		-	Function pointer to compare node items.
 *	btr_node_typ **pq	-	Reference to the current node.
 *	btr_node_typ **ps	-	Reference to the node where balancing begins.
 *	btr_node_typ **pt	-	Reference to the parent of ps.
 *
 *	Initial addition of the node to the tree which sets the utility 
 *	pointers for the AVL rotation which will occur later.
 *
 *	RETURN
 *		FALSE	-	if existing item is found at pnode.
 *		ERROR	-	If memory error, or tree problem.
 *		TRUE	-	if item is inserted, and the current node,
 *					the balance node, and its parent are set.
 */

static int avl_insert( pitem, pstart, pfcmp, pq, ps, pt )
void *pitem;
btr_node_typ *pstart;
int (*pfcmp)();
register btr_node_typ **pq;			/*	The current node					*/
register btr_node_typ **ps;			/*	The balancing node					*/
register btr_node_typ **pt;			/*	The parent of the balance node		*/
{
	register int result;
	int found;
	btr_node_typ *pp;

	*pt = pstart;
	*ps = pstart;
	pp = pstart;

	while( pp != NULL )
	{
		result = (*pfcmp)( pitem, pp->pitem );

		if( result == 0 )					/*	Exit with a match	*/
		{
			*pq = pp;
			return( FALSE );
		}

		if( result < 0 )
		{
			if( (*pq = pp->pleft) == NULL )
			{
				if( (*pq = btr_inst_node( pitem )) == NULL )
					return( ERROR );
				else
				{
					pp->pleft = *pq;
					return( TRUE );
				}
			}
			else if( (*pq)->balance != AVL_BALANCED )
			{
				*pt = pp;
				*ps = *pq;
			}
		}
		else
		{
			if( (*pq = pp->pright) == NULL )
			{
				if( (*pq = btr_inst_node( pitem )) == NULL )
					return( ERROR );
				else
				{
					pp->pright = *pq;
					return( TRUE );
				}
			}
			else if( (*pq)->balance != AVL_BALANCED )
			{
				*pt = pp;
				*ps = *pq;
			}
		}
		pp = *pq;
	}
	return( ERROR );
}

/**	SYN/OPSIS
 *
 *	#include <sys_btr.h>
 *
 *	static int avl_balance( pitem, pfcmp, pq, pr, ps, pfactor )
 *
 *	void *pitem			-	Pointer to the new data item.
 *	int (*pfcmp)()		-	Function pointer to compare node items.
 *	btr_node_typ *pq	-	Reference to the current node.
 *	btr_node_typ **pr	-	Reference to the node for rotation.
 *	btr_node_typ *pt	-	Reference to the parent of ps.
 *
 *	Maintains the balance factors of the subtree, and sets
 *	the node for rotation and the balance factor for the subtree..
 *
 *	RETURN
 *		AVL_DONE		-	if tree is balanced.
 *		AVL_SINGLE		-	if single rotation is necessary.
 *		AVL_DOUBLE		-	if double rotation is necessary.
 */

static int avl_balance( pitem, pfcmp, pq, pr, ps, pfactor )
void *pitem;
int (*pfcmp)();
register btr_node_typ *pq;			/*	The current node.					*/
btr_node_typ **pr;					/*	The rotation node.					*/
register btr_node_typ *ps;			/*	The balancing node.					*/
int *pfactor;
{
	register int result;
	register btr_node_typ *pp;			/*	Pointer for descent.	*/

	if( (*pfcmp)( pitem, ps->pitem ) < 0 )
		pp = ps->pleft;
	else
		pp = ps->pright;

	*pr = pp;

	while( pp != pq )
	{
		result = (*pfcmp)( pitem, pp->pitem );

		if( result < 0 )
		{
			pp->balance = AVL_LEFT;
			pp = pp->pleft;
		}
		else if( 0 < result )
		{
			pp->balance = AVL_RIGHT;
			pp = pp->pright;
		}
		else
			break;
	}
	if( (*pfcmp)( pitem, ps->pitem ) < 0 )
		*pfactor = AVL_LEFT;
	else
		*pfactor = AVL_RIGHT;

	if( ps->balance == AVL_BALANCED )
	{
		ps->balance = *pfactor;
		return( AVL_DONE );
	}

	if( ps->balance == *pfactor )
	{
		if( (*pr)->balance == *pfactor )
			return( AVL_SINGLE );
		else
			return( AVL_DOUBLE );
	}
	else 
	{
		ps->balance = AVL_BALANCED;
		return( AVL_DONE );
	}
}

/**	SYN/OPSIS
 *	#include <sys_btr.h>
 *
 *	static btr_node_typ *avl_single( factor, ps, pr )
 *
 *	int factor			-	The subtree balance factor.
 *	btr_node_typ *pr	-	The node for rotation.
 *	btr_node_typ *ps	-	The node for the subtree.
 *
 *	Performs a single AVL rotation, and returns the new subtree root.
 *
 *	RETURN
 *		pointer to the new root of the balanced subtree.
 */

static btr_node_typ *avl_single( factor, ps, pr )
int factor;
btr_node_typ *ps, *pr;
{
	btr_node_typ *pp;

	pp = pr;
	if( factor == AVL_RIGHT )
	{
		ps->pright = LINK( -factor, pr );
		pr->pleft = ps;
	}
	else
	{
		ps->pleft = LINK( -factor, pr );
		pr->pright = ps;
	}
	ps->balance = AVL_BALANCED;
	pr->balance = AVL_BALANCED;
	return( pp );
}

/**	SYN/OPSIS
 *	#include <sys_btr.h>
 *
 *	static btr_node_typ *avl_double( factor, ps, pr )
 *
 *	int factor			-	The subtree balance factor.
 *	btr_node_typ *pr	-	The node for rotation.
 *	btr_node_typ *ps	-	The node for the subtree.
 *
 *	Performs a double AVL rotation, and returns the new subtree root.
 *
 *	RETURN
 *		pointer to the new root of the balanced subtree.
 */

static btr_node_typ *avl_double( factor, ps, pr )
int factor;
btr_node_typ *ps, *pr;
{
	btr_node_typ *pp;

	pp = LINK( -factor, pr );
	if( factor == AVL_RIGHT )
	{
		pr->pleft = LINK( factor, pp );
		pp->pright = pr;
		ps->pright = LINK( -factor, pp );
		pp->pleft = ps;
	}
	else
	{
		pr->pright = LINK( factor, pp );
		pp->pleft = pr;
		ps->pleft = LINK( -factor, pp );
		pp->pright = ps;
	}

	if( pp->balance == factor )
	{
		ps->balance =  -factor;
		pr->balance = AVL_BALANCED;
	}
	else if( pp->balance == AVL_BALANCED )
	{
		ps->balance = AVL_BALANCED;
		pr->balance = AVL_BALANCED;
	}
	else
	{
		ps->balance = AVL_BALANCED;
		pr->balance = factor;
	}
	pp->balance = AVL_BALANCED;
	return( pp );
}

/**	SYN/OPSIS
 *	
 *	#include <sys_btr.h>
 *
 *	DESCRIPTION
 *
 *	void *btr_find( void *pitem, btr_node_typ **pparent, int (*pfcmp)() )
 *	pitem	-	Pointer to tree item to be found.
 *	pparent	-	Pointer to pointer to root node of tree.
 *	pfcmp	-	Pointer to user supplied comparison function of tree nodes.
 *	
 *	Find a tree node which matches the given item, and return a
 *	pointer to it.
 *
 *	RETURN
 *		NULL		if parent or search item is invalid, or item is not found.
 *		non-NULL	Pointer to matching item.
 */

void *btr_find( pitem, pparent, pfcmp )
void *pitem;
register btr_node_typ **pparent;
int (*pfcmp)();
{
	register int result;

	if( (pitem == NULL) || (pparent == NULL) )		 	/*	Bad arguments.	*/
		return( NULL );

	if( *pparent == NULL )							/*	Found a leaf.		*/
		return( NULL );

	if( (*pparent)->pitem == NULL )			/*	Found an empty node.		*/
		return( NULL );

	result = (*pfcmp)( pitem, (*pparent)->pitem );

	if( result == 0 )
		return( (*pparent)->pitem );
	else if( result < 0 )
		return( btr_find( pitem, &((*pparent)->pleft), pfcmp ) );
	else
		return( btr_find( pitem, &((*pparent)->pright), pfcmp ) );
}

/**	SYN/OPSIS
 *	
 *	#include <sys_btr.h>
 *
 *	DESCRIPTION
 *
 *	void *btr_rm( void *pitem, btr_node_typ **proot, int (*pfcmp)() )
 *	pitem	-	Pointer to tree item to be removed.
 *	proot	-	Pointer to pointer to root node of tree.
 *	pfcmp	-	Pointer to user supplied comparison function of tree nodes.
 *	
 *	RETURN
 *		NULL		if root or search item is invalid, or item is not found.
 *		non-NULL	Pointer to matching item.
 */

void *btr_rm( pitem, proot, pfcmp )
void *pitem;
register btr_node_typ **proot;
int (*pfcmp)();
{
	return( NULL );
}


/**	SYN/OPSIS
 *	#include <sys_list.h>
 *	#include <sys_btr.h>
 *
 *	void btr_destroy( ppnode, pfitem )
 *	btr_node_typ **ppnode		root of the tree to be destroyed.
 *	void (*pfitem)()			function to be performed on every item
 *								(do not free the item).
 *
 *	DESCRIPTION
 *	Given the root, traverses the tree (left to right, depth first).  Whenever 
 *	the current node is a leaf, user function call is performed on the item, 
 *	the item is destroyed, the leaf is destroyed and the pointer is set to NULL. 
 */

void btr_destroy( ppnode, pfitem )
register btr_node_typ **ppnode;
void (*pfitem)();
{
	btr_node_typ *pnode;

	pnode = *ppnode;

	if( pnode == NULL )							
		return;

 	/* BTR_PREORDER */
	if( pnode->pleft != NULL )						
		btr_destroy( &(pnode->pleft), pfitem );

 	/* BTR_INORDER */
	if( pnode->pright != NULL )					
		btr_destroy( &(pnode->pright), pfitem );

 	/* BTR_POSTORDER  && BTR_LEAF */
	if( pfitem != NULL )
		(*pfitem)( pnode->pitem ); 			/* Call user function. */ 

	if( pnode->pitem != NULL )
	{
		free( pnode->pitem );
		pnode->pitem = NULL;
	}

	free( pnode );
	*ppnode = NULL;

	return;
}

/**	SYN/OPSIS
 *	
 *	#include <sys_btr.h>
 *
 *	DESCRIPTION
 *
 *	void btr_walk( btr_node_typ *proot, void (*pfitem)() )
 *	proot	-Pointer to root node of tree.
 *	pfitem	-Pointer to user supplied function which will be
 *		called at each node.  The function should have the
 *		prototype:
 *
 *		void (*pfitem)( void *pitem, btr_visit_typ order, int level );
 *
 *		where:
 *			pitem	-Pointer to the node data item.
 *			order	-Order in which this node is being visited.
 *				Order is defined to indicate the
 *				evaluation of the current node:
 *				BTR_PREORDER 		before the left node,
 *				BTR_INORDER/LEAF 	after the left node,
 *				BTR_POSTORDER		after the right node.
 *
 *			level	-Depth of the current item, root being zero.
 *	
 *	Performs a depth-first, left-to-right traversal of the binary tree.
 *	The user function is called with the indicated arguments each of
 *	the three times that a node is visited during the traversal.
 *	The traversal fails if a node item is NULL, or if a node pointer
 *	is NULL.
 *
 *	RETURN
 *		none
 */

void btr_walk( proot, pfitem )
register btr_node_typ *proot;
void (*pfitem)();
{
	if( pfitem == NULL )
		return;

	btr_depth_left( proot, item_func, pfitem, 0 );
}

void btr_dump( proot, pfitem )
register btr_node_typ *proot;
void (*pfitem)();
{
	btr_depth_left( proot, dump_node, pfitem, 0 );
}

static void dump_node( pnode, pfitem, order, level )
btr_node_typ *pnode;
void (*pfitem)();
btr_visit_typ order;
int level;
{
	int i;

	if( (order == BTR_INORDER) || (order == BTR_LEAF) )
	{
		for( i = 0; i < level; i++ )
			printf( "\t" );
		printf( "0x%06lx 0x%06lx 0x%06lx %2d\t", (long) pnode, 
			(long) pnode->pleft, (long) pnode->pright, pnode->balance );
	}
	(*pfitem)( pnode->pitem, order, level );
}

/*
 *	Simple node walk function that only calls the
 *	user's item function.
 */

static void item_func( pnode, pfitem, order, level )
btr_node_typ *pnode;
void (*pfitem)();
btr_visit_typ order;
int level;
{
	(*pfitem)( pnode->pitem, order, level );
}

/**	SYN/OPSIS
 *
 *	#include <sys_btr.h>
 *
 *	DESCRIPTION
 *
 *	static void btr_depth_left( btr_node_typ *pnode, 
 *		void (*pfnode), void (*pfitem)(), int level );
 *
 *	pnode	-Pointer to the current node.
 *	pfnode	-Pointer to function called at each node.
 *	pfitem	-Pointer to function called at each item.
 *	level	-Depth of the current node, with the tree root being level zero.
 *	
 *	Left-to-right, depth-first traversal of a binary tree,
 *	which calls a function for the node, and for the item.
 */

static void btr_depth_left( pnode, pfnode, pfitem, level )
register btr_node_typ *pnode;
void (*pfnode)();
void (*pfitem)();
int level;
{
	if( pnode == NULL )								 	/*	Bad argument.	*/
		return;

	if( pnode->pitem == NULL )	/*	Found an empty node.	*/
		return;

 	(*pfnode)( pnode, pfitem, BTR_PREORDER, level );
	if( pnode->pleft != NULL )
		btr_depth_left( pnode->pleft, pfnode, pfitem, level+1 );

	if( (pnode->pleft == NULL) && (pnode->pright == NULL) )
 		(*pfnode)( pnode, pfitem, BTR_LEAF, level );
	else
 		(*pfnode)( pnode, pfitem, BTR_INORDER, level );

	if( pnode->pright != NULL )
		btr_depth_left( pnode->pright, pfnode, pfitem, level+1 );

 	(*pfnode)( pnode, pfitem, BTR_POSTORDER, level );
}
