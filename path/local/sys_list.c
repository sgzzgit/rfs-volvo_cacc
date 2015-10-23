/**\file	
 *	sys_list.c
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *	Linked list utilities.
 *
 */

#include <sys_os.h>
#include "sys_mem.h"
#include "sys_lib.h"
#include "sys_list.h"

static ll_node_typ *ll_inst_node( void *pitem, ll_node_typ *pnext );
static dl_node_typ *dl_inst_node( void *pitem,
	dl_node_typ *pprev, struct dl_node_str *pnext );
extern bool_typ dl_insert_before( dl_head_typ *phead, dl_node_typ *pcurrent,
	void *pitem );

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	ll_head_typ *ll_create( void )
 *
 *	DESCRIPTION
 *	This function creates the head for a single linked list.
 *
 *	RETURN
 *		phead	-	pointer to the head of the single linked list.
 *		NULL	-	if memory space is unavailable.
 */	

ll_head_typ *ll_create()
{
	ll_head_typ *phead;

	if( ( phead = (void *) malloc( sizeof( ll_head_typ ) ) ) != NULL )
	{
		phead->length = 0;
		phead->pfirst = NULL;
		phead->plast = NULL;
		return( phead );
	}
	else
		return( NULL );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	ll_node_typ *ll_inst_node( pitem, pnext )
 *	void *pitem		-	item for inserting into the linked list
 *	ll_node_typ *pnext	-	pointer to the node which is to follow
 *
 *	DESCRIPTION
 *	This function inserts a node.
 *
 *	RETURN
 *		pnode	-pointer to the new node in the single linked list
 *		NULL	-if memory space is unavailable
 */	

static ll_node_typ *ll_inst_node( pitem, pnext )
void *pitem;
ll_node_typ *pnext;
{
	register ll_node_typ *pnode;

	if( ( pnode = (void *) malloc( sizeof( ll_node_typ ) ) ) != NULL )
	{
		pnode->pitem = pitem;
		pnode->pnext = pnext;
		return( pnode );
	}
	else
		return( NULL );
}

/**	SYNOPSIS
 *
 *	void ll_free( phead )
 *	ll_head_typ *phead	-pointer to the head of a single linked list
 *
 *	DESCRIPTION
 *	This function destroys a single linked list of simple objects and 
 *	frees the associated memory. 
 *
 *	RETURN
 *		Nothing		
 */	

void ll_free( phead )
register ll_head_typ *phead;
{
	void *pitem;

	if( phead == NULL ) 
		return;

	if( phead->length == 0 )
	{
 		free( phead );
		return;
	}

	while( (pitem = ll_rm_last( phead )) != NULL )
		free( pitem );

	free( phead );
}



/* SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	ll_node_typ *ll_first( phead, pkey, pfkeycmp )
 *	ll_head_typ *phead	- Head of the list to be searched.
 *	void *pkey		-Pointer to key which is to be matched.
 *	int (*pfkeycmp)()	-User supplied function which compares two
 *				list items.  pfcmp should return less 
 *				than zero, zero, or greater than zero
 *				depending on the relative magnitude
 *				of the left and right parameters.
 *
 *	DESCRIPTION
 *	ll_first() linearly searches the elements of a list for a matching
 *	key.  The user must supply a function which is used to match the
 *	key with the list elements.
 *
 *	ll_first() can be used to maintain a static pointer to a current
 *	node which can be used by the ll_next() function.  The user should 
 *	be aware that  this node element could be modified by an intervening
 *	list operation.
 *
 *	RETURN
 *		non-NULL	-Pointer to the node which matches the key.
 *		NULL		-If the key is not matched in any list item.
 */

ll_node_typ *ll_first( phead, pkey, pfkeycmp )
ll_head_typ *phead;	
register void *pkey;
register int (*pfkeycmp)();
{
	register ll_node_typ *pnode;

	if( phead == NULL )
		return( NULL );

	for( pnode = phead->pfirst; pnode != NULL; pnode = pnode->pnext )
	{
		if( ((*pfkeycmp)( pkey, pnode->pitem ) ) == 0 )
			return( pnode );
	}
	return( NULL );
}


/* SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	ll_node_typ *ll_next( pnode, pkey, pfkeycmp )
 *	ll_node *pnode		-Pointer to the current node.	
 *	void *pkey		-Pointer to key which is to be matched.
 *	int (*pfkeycmp)()	-User supplied function which compares two
 *				list items.  pfcmp should return less 
 *				than zero, zero, or greater than zero
 *				depending on the relative magnitude
 *				of the left and right parameters.
 *
 *	DESCRIPTION
 *	ll_next() linearly searches the elements of the list which has
 *	first been searched by ll_first() for a matching key.
 *	The user must supply a function which is used to match the
 *	key with the list elements.
 *
 *	The return from ll_next() can be used to maintain a pointer to
 *	a current node which can be used in subsequent calls to the
 *	ll_next() function.
 *
 *	The validity of this pointer could be compromised by a number
 *	of list intervening operations. 
 *
 *	RETURN
 *		non-NULL	-Pointer to the node which matches the key.
 *		NULL		-If the key is not matched in any list item.
 */
 
ll_node_typ *ll_next( pnode, pkey, pfkeycmp )
ll_node_typ *pnode;	
register void *pkey;
register int (*pfkeycmp)();
{
	register ll_node_typ *pcurr_node;

	if( pnode == NULL )
		return( NULL );

	for( pcurr_node = pnode; pcurr_node != NULL; 
		pcurr_node = pcurr_node->pnext )
	{
		if( ((*pfkeycmp)( pkey, pcurr_node->pitem ) ) == 0 )
			return( pcurr_node );
	}

	return( NULL );
}




/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	int ll_insert( pitem, phead )
 *	void *pitem		-item for inserting into the linked list
 *	ll_head_typ *phead	-pointer to the head of the linked list
 *
 *	DESCRIPTION
 *	This function inserts a node at the front of a linked list.
 *
 *	RETURN
 *		TRUE	-	if successful
 *		FALSE	-	if memory space is unavailable.
 */	

int ll_insert( pitem, phead )
void *pitem;
register ll_head_typ *phead;
{
	ll_node_typ *pnew;

	if( (pnew = ll_inst_node( pitem, phead->pfirst )) != NULL )
	{
		phead->pfirst = pnew;
		if( phead->length == 0 )
			phead->plast = pnew;
		phead->length++;
		return( TRUE );
	}
	else
		return( FALSE );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	int ll_append( pitem, phead )
 *	void *pitem		-item for appending to the linked list
 *	ll_head_typ *phead	-pointer to the head of the linked list
 *
 *	DESCRIPTION
 *	This function appends a node to the end of a linked list.
 *
 *	RETURN
 *		TRUE	-	if successful
 *		FALSE	-	if it was unable to append a node to the end of
 *						a linked list
 */	

int ll_append( pitem, phead )
void *pitem;
register ll_head_typ *phead;
{
	ll_node_typ *pnew;

	if( ( pnew = ll_inst_node( pitem, NULL )) != NULL )
	{
		if( phead->length == 0 )
			phead->pfirst = pnew;
		else
			phead->plast->pnext = pnew;

		phead->plast = pnew;
		phead->length++;
		return( TRUE );
	}
	else
		return( FALSE );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	void *ll_rm_first( phead )
 *	ll_head_typ *phead	-pointer to the head 
 *				of the linked list
 *
 *	DESCRIPTION
 *	This function deletes the first node in a linked list and 
 *	frees its resources.
 *
 *	RETURN
 *		A pointer to the item which was removed.
 */	

void *ll_rm_first( phead )
register ll_head_typ *phead;
{
	ll_node_typ *pold;
	void *pitem;

	if( (phead == NULL) || (phead->pfirst == NULL) )
		return( NULL );

	pold = phead->pfirst;
	pitem = phead->pfirst->pitem;
	
	phead->pfirst = pold->pnext;
	phead->length--;
	if( phead->length == 0 )
		phead->plast = NULL;

	free( (void *) pold );
	return( pitem );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	void *ll_rm_last( phead )
 *	ll_head_typ *phead	-pointer to the head of the linked list
 *
 *	DESCRIPTION
 *	This function deletes the last node in a linked list.
 *
 *	RETURN
 *		A pointer to the item which was deleted from the linked list.
 *		NULL	-if the list is empty
 */	

void *ll_rm_last( phead )
register ll_head_typ *phead;
{
	register ll_node_typ *ptemp;
	ll_node_typ *pold;
	void *pitem;
	int i;
	
	if( phead->length == 0 )
		return( NULL );
	pold = phead->plast;						/* 		to be freed		    */
	pitem = pold->pitem;
	ptemp = phead->pfirst;
	if( phead->length > 1 )
	{
		for( i = 0; i < (phead->length - 2); i++ )
			ptemp = ptemp->pnext;		
		ptemp->pnext = NULL;
		phead->plast = ptemp;
	}
	else
	{
		phead->pfirst = NULL;
		phead->plast = NULL;
	}
	phead->length--;
	free( (void *) pold );	
	return( pitem ); 
}


/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	void *ll_rm_node( phead, pcurrent, pprev )
 *	ll_head_typ *phead	-	pointer to the head of the linked list
 *	ll_node_typ **pcurrent	-	pointer to the node for	removal
 *	ll_node_typ *pprev	-	pointer to the previous node 
 *
 *	DESCRIPTION
 *	This function deletes a node and sets the current node pointer to 
 *	the previous node or NULL if the previous node was the first in 
 *	the list.
 *
 *	RETURN
 *		pitem	-	pointer to the item which was removed
 *		NULL	-	if the node was the first one  
 */	

void *ll_rm_node( phead, pcurrent, pprev )
register ll_head_typ *phead;
ll_node_typ **pcurrent;
ll_node_typ *pprev;
{
	void *pitem;
	ll_node_typ *pold;
	
	pold = *pcurrent;
	*pcurrent = pprev;

	if( (pold == NULL) || (phead == NULL) ||
		(phead->pfirst == NULL) )
	{
		return( NULL );
	}
	
	if( pprev == NULL )
	{
		pitem = ll_rm_first( phead );
		return( pitem );
	}
	else if( pold->pnext == NULL )
	{
		pitem = ll_rm_last( phead );
		return( pitem );
	}

	pitem = pold->pitem;
	pprev->pnext = pold->pnext;

	phead->length--;
	free( (void*) pold );
	return( pitem );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	int ll_length( ll_head_typ *phead );
 *	ll_head_typ *phead	-	Pointer to the list.
 *
 *	DESCRIPTION
 *	Get the length of the list, given the head.
 *
 *	RETURN
 *		ERROR		-	Pointer to head is NULL.
 *		non-ERROR	-	Number of items in list.
 *
 */

int ll_length( phead )
ll_head_typ *phead;
{
    if( phead == NULL )
        return( ERROR );
    else
        return( phead->length );
}


/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	dl_head_typ *dl_create( void )
 *
 *	DESCRIPTION
 *	This function creates the head for a double linked list.
 *
 *	RETURN
 *		phead	-	if successful
 *		NULL	-	if the list the list cannot be created
 */	

dl_head_typ *dl_create()
{
	register dl_head_typ *phead;

	if( ( phead = (void *) malloc( sizeof( dl_head_typ ) ) ) != NULL )
	{
		phead->length = 0;
		phead->pfirst = NULL;
		phead->plast = NULL;
		return( phead );
	}
	else
		return( NULL );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	void dl_free( phead )
 *	dl_head_typ *phead	-pointer to the head of a 
 *				double linked list
 *
 *	DESCRIPTION
 *	This function destroys a double linked list of simple objects and 
 *	frees the associated memory. 
 *
 *	RETURN
 *		Nothing		
 */	

void dl_free( phead )
register dl_head_typ *phead;
{
	void *pitem;

	if( phead == NULL ) 
		return;

	if( phead->length == 0 )
	{
 		free( phead );
		return;
	}

	while( (pitem = dl_rm_last( phead )) != NULL )
		free( pitem );

	free( phead );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	dl_node_typ *dl_inst_node( pitem, pprev, pnext )
 *	void *pitem						-	pointer to the item for inserting 
 *	dl_node_typ *pprev	-pointer to the node prior to the
 *											item for  inserting
 *	dl_node_typ *pnext	-pointer to the node after the item
 *											for inserting
 *
 *	DESCRIPTION
 *	This function inserts a node into a double linked list.
 *
 *	RETURN
 *		pnode	-	on success
 *		NULL	-	if allocating the memory space fails
 */	

static dl_node_typ *dl_inst_node( pitem, pprev, pnext )
void *pitem;
dl_node_typ *pprev, *pnext;
{
	register dl_node_typ *pnode;

	if( ( pnode = (void *) malloc( sizeof( dl_node_typ ) ) ) != NULL )
	{
		pnode->pitem = pitem;
		pnode->pprev = pprev;
		pnode->pnext = pnext;
		return( pnode );
	}
	else
		return( NULL );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	int dl_insert( pitem, phead )
 *	void *pitem				-	item to be inserted
 *	dl_head_typ *phead		-	pointer to the head of the double
 *									linked list
 *
 *	DESCRIPTION
 *	This function inserts a node at the front of a double linked list.
 *
 *	RETURN
 *		TRUE	-	on success
 *		FALSE	-	if allocating the memory space fails
 */	

int dl_insert( pitem, phead)
void *pitem;
register dl_head_typ *phead;
{
	dl_node_typ *pnew;

	if( (pnew = dl_inst_node( pitem, NULL, phead->pfirst )) != NULL )
	{
		if( phead->length == 0 )
			phead->plast = pnew;
		else
			phead->pfirst->pprev = pnew;

		phead->pfirst = pnew;
		phead->length++;
		return( TRUE );
	}
	else
		return( FALSE );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	int dl_append( pitem, phead )
 *	void *pitem					-	item to be appended to the list 
 *	dl_head_typ *phead	-	pointer to the head of the double
 *										linked list
 *	DESCRIPTION
 *	This function appends a node to the end of a double linked list.
 *
 *	RETURN
 *		TRUE	-	on success
 *		FALSE	-	if allocating the memory space fails
 */	

int dl_append( pitem, phead )
void *pitem;
register dl_head_typ *phead;
{
	dl_node_typ *pnew;

	if( ( pnew = dl_inst_node( pitem, phead->plast, NULL )) != NULL )
	{
		if( phead->length == 0 )
			phead->pfirst = pnew;
		else
			phead->plast->pnext = pnew;

		phead->plast = pnew;
		phead->length++;
		return( TRUE );
	}
	else
		return( FALSE );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	void *dl_rm_first( phead )
 *	dl_head_typ *phead	-	pointer to the head of the double
 *										linked list
 *	DESCRIPTION
 *	This function removes the first entry in a double linked list.	
 *
 *	RETURN
 *		pitem	-	pointer to the item which was removed	
 */	

void *dl_rm_first( phead )
register dl_head_typ *phead;
{
	dl_node_typ *pold;
	void *pitem;

	if( (phead == NULL) || (phead->pfirst == NULL) )
		return( NULL );

	pold = phead->pfirst;
	pitem = phead->pfirst->pitem;
	
	phead->pfirst = pold->pnext;
	phead->length--;
	if( phead->length == 0 )
		phead->plast = NULL;
	else
		phead->pfirst->pprev = NULL;

	free( (void *) pold );
	return( pitem );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	void *dl_rm_last( phead )
 *	dl_head_typ *phead	-	pointer to the head of the double
 *										linked list
 *	DESCRIPTION
 *	This function removes the last entry in a double linked list.	
 *
 *	RETURN
 *		pitem	-	pointer to the item which was removed
 *		NULL	-	if empty list
 */	

void *dl_rm_last( phead )
register dl_head_typ *phead;
{
	dl_node_typ *pold;
	void *pitem;

	if( (phead->length) == 0 )
		return( NULL );
		
	pold = phead->plast;
	pitem = phead->plast->pitem;
	
	phead->plast = pold->pprev;
	phead->length--;
	if( phead->length == 0 )
		phead->pfirst = NULL;
	else
		phead->plast->pnext = NULL;

	free( (void *) pold );
	return( pitem );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	void *dl_rm_node( phead, pcurrent )
 *	dl_head_typ *phead		-	pointer to the head of the 
 *											double linked list
 *	dl_node_typ **pcurrent	-	pointer to the node for 
 *											removal
 *
 *	DESCRIPTION
 *	This function deletes a node and sets the current node
 *	pointer to the previous node or NULL if the node was the
 *	first in the double linked list.
 *
 *	RETURN
 *		pitem	-	pointer to the item which was removed
 */	

void *dl_rm_node( phead, pcurrent )
register dl_head_typ *phead;
dl_node_typ **pcurrent;
{
	void *pitem;
	dl_node_typ *pold;

	pold = *pcurrent;
	*pcurrent = pold->pprev;

	if( pold->pprev == NULL )
	{
		pitem = dl_rm_first( phead );
		*pcurrent = NULL;
		return( pitem );
	}
	else if( pold->pnext == NULL )
	{
		pitem = dl_rm_last( phead );
		return( pitem );
	}

	pitem = pold->pitem;

	pold->pprev->pnext = pold->pnext;
	pold->pnext->pprev = pold->pprev;
		
	phead->length--;
	free( (void *) pold );
	return( pitem );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	dl_node_typ *dl_nth_node( n, phead )
 *	int n							-	n is offset into the double 
 *											linked list
 *	dl_head_typ *phead		-	pointer to the head of the 
 *											double linked list
 *
 *	DESCRIPTION
 *	This function returns a pointer to the node which is "n" 
 *	nodes from the start of the double linked list.
 *
 *	RETURN
 *		pitem	-	pointer to the item which was removed 
 *		NULL	-	if "n" is an invalid value:	
 *						n < 0  
 *						the list length <= n
 */	

dl_node_typ *dl_nth_node( n, phead )
int n;
dl_head_typ *phead;
{
	register dl_node_typ *pnode;
	
	/* Length count starts from 1, but n = 0 is the first item	*/

	if( (n < 0) || ( phead->length <= n ) )
		return( NULL );

	pnode = phead->pfirst;
	while( --n >= 0 )
	{
		if( pnode == NULL )
			return( NULL );
		else
			pnode = pnode->pnext;
	}
	return( pnode );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	int dl_mv_node( pitem, phead, size, maximum )
 *	void *pitem					-	item for copying into a buffer
 *	dl_head_typ *phead	-	pointer to the head of the 
 *										double linked list
 *	unsigned size				-	size of element for copying
 *	int maximum					-	element offset into the double 
 *										linked list
 *	DESCRIPTION
 *	This function is a potentially destructive copy of an item 
 *	into a buffer. It inserts from the front and deletes off the 
 *	back if there are too many items in the queue.
 *	
 *	RETURN
 *		double linked list length
 *		FALSE			-	if unable to allocate storage for the 
 *							item to be stored or if inserting the 
 *							item fails.
 *	BUGS
 *	Potentially destructive copy of an item into a buffer.
 *	Assumes that deallocating the old item is sufficient.
 *	THIS WON'T WORK WITH A LIST OF LISTS, FOR EXAMPLE.
 */	

int dl_mv_node( pitem, phead, size, maximum )
void *pitem;
dl_head_typ *phead;
unsigned size;
int maximum;
{
	void *psave;
	void *pold;

	if( (psave = ( void * ) malloc( size )) == NULL )
	{
		return( FALSE );
	}
	else
	{
		bytecopy( size, (char *) pitem, (char *) psave );

		if( dl_insert( psave, phead ) == FALSE )
			return( FALSE );

		if( phead->length > maximum )
		{
			pold = dl_rm_last( phead );
			if( pold != NULL )
				free( pold );
		}
	}
	return( phead->length );
}

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	int dl_nth_insert( n, pitem, phead )
 *	int n						-	node offset into the double 
 *										linked list
 *	void *pitem					-	item for inserting into 
 *										the double linked list	
 *	dl_head_typ *phead	-	pointer to the head of the 
 *										double linked list
 *	DESCRIPTION
 *	This function inserts an item into the "nth" position in
 *	a double linked list.
 *	
 *	RETURN
 *		TRUE			-	if it succeeds in:
 *								inserting the first item 
 *									into the list	
 *								inserting the item into the 
 *									middle of the list
 *								appending the item to the 
 *									end of the list
 *		FALSE			-	if the search for the nth node fails 	
 *						-	if the search for the nth-1 node fails 
 *						-	if inserting the node fails
 */	
int dl_nth_insert( n, pitem, phead ) 
int n;
void *pitem;
register dl_head_typ *phead;
{
   dl_node_typ *pnew_left;
   dl_node_typ *pnew;
   dl_node_typ *pnew_right;

   if( n == 0 )
      return( dl_insert( pitem, phead ) );

   if( n >= phead->length )
      return( dl_append( pitem, phead ) );

   if( (pnew_right = dl_nth_node( n, phead )) == NULL )
      return(FALSE);

   if( (pnew_left = dl_nth_node( n-1, phead )) == NULL )
      return(FALSE);

   if( ( pnew = dl_inst_node( pitem, pnew_left,
					 pnew_right )) != NULL )
   {
      pnew_left->pnext = pnew;
      pnew_right->pprev = pnew;
   }
   else
      return(FALSE);

   phead->length++;
   return(TRUE);
}


/* SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	dl_node_typ *dl_first( phead, pkey, pfkeycmp )
 *	dl_head_typ *phead		- Head of the list to be searched.
 *	void *pkey				-	Pointer to key which is to be matched.
 *	int (*pfkeycmp)()		-	User supplied function which compares two
 *								list items.  pfcmp should return less 
 *								than zero, zero, or greater than zero
 *								depending on the relative magnitude
 *								of the left and right parameters.
 *
 *	DESCRIPTION
 *	dl_first() linearly searches the elements of a list for a matching
 *	key.  The user must supply a function which is used to match the
 *	key with the list elements.
 *
 *	dl_first() can be used to maintain a static pointer to a current
 *	node which can be used by the dl_next() function.  The user should 
 *	be aware that  this node element could be modified by an intervening
 *	list operation.
 *
 *	RETURN
 *		non-NULL	-	Pointer to the node which matches the key.
 *		NULL		-	If the key is not matched in any list item.
 */
 
dl_node_typ *dl_first( phead, pkey, pfkeycmp )
dl_head_typ *phead;	
register void *pkey;
register int (*pfkeycmp)();
{
	register dl_node_typ *pnode;

	if( phead == NULL )
		return( NULL );

	for( pnode = phead->pfirst; pnode != NULL; pnode = pnode->pnext )
	{
		if( ((*pfkeycmp)( pkey, pnode->pitem ) ) == 0 )
			return( pnode );
	}
	return( NULL );
}

/* SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	dl_node_typ *dl_next( pnode, pkey, pfkeycmp )
 *	dl_node *pnode			-	Pointer to the current node.	
 *	void *pkey				-	Pointer to key which is to be matched.
 *	int (*pfkeycmp)()		-	User supplied function which compares two
 *								list items.  pfcmp should return less 
 *								than zero, zero, or greater than zero
 *								depending on the relative magnitude
 *								of the left and right parameters.
 *
 *	DESCRIPTION
 *	dl_next() linearly searches the elements of the list which has
 *	first been searched by dl_first() for a matching key.
 *	The user must supply a function which is used to match the
 *	key with the list elements.
 *
 *	The return from dl_next() can be used to maintain a pointer to
 *	a current node which can be used in subsequent calls to the
 *	dl_next() function.
 *
 *	The validity of this pointer could be compromised by a number
 *	of intervening list operations. 
 *
 *	RETURN
 *		non-NULL	-	Pointer to the node which matches the key.
 *		NULL		-	If the key is not matched in any list item.
 */
 
dl_node_typ *dl_next( pnode, pkey, pfkeycmp )
dl_node_typ *pnode;	
register void *pkey;
register int (*pfkeycmp)();
{
	register dl_node_typ *pcurr_node;

	if( pnode == NULL )
		return( NULL );

	for( pcurr_node = pnode; pcurr_node != NULL; 
		pcurr_node = pcurr_node->pnext )
	{
		if( ((*pfkeycmp)( pkey, pcurr_node->pitem ) ) == 0 )
			return( pcurr_node );
	}

	return( NULL );
}

/**	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	bool_typ dl_pq_add( phead, pitem, pfcmp, size )
 *	dl_head_typ *phead	-	Pointer to the head of the double linked list. 
 *	void *pitem			-	Item for copying into a buffer.
 *	int (*pfcmp)()		-	User supplied function which compares two
 *							list items.  pfcmp should return less 
 *							than zero, zero, or greater than zero
 *							depending on the relative magnitude
 *							of the left and right parameters.
 *	unsigned size		-	size of element for copying in bytes.
 *
 *	DESCRIPTION
 *	This function inserts a node into a double linked list in descending
 *	order.  The list is ordered according to (*pfcmp)().
 *	The node item with size of size is a copy of the pitem.
 *	The user must supply a function which is used to compare pitem with 
 *	the list elements.
 *
 *	RETURN
 *		TRUE	if the item is successfully inserted into the list.
 *		FALSE	if unable to allocate storage for the item 	
 *				or if inserting the item fails.
 *
 */

bool_typ dl_pq_add( phead, pitem, pfcmp, size )
register dl_head_typ *phead;
void *pitem;
int(*pfcmp)();
unsigned size;
{
	register dl_node_typ *pnode;
	void *psave;

	if( phead == NULL )
		return( FALSE );

	if( (psave = (void *) malloc( size )) == NULL )
		return( FALSE );

	bytecopy( size, (char *) pitem, (char *) psave );
	
	for( pnode = phead->pfirst; pnode != NULL; pnode = pnode->pnext )
	{
		if( ( (*pfcmp)( psave, pnode->pitem ) ) > 0 )
		{
			if( dl_insert_before( phead, pnode, psave ) != TRUE )
			{
				free( psave );
				return( FALSE );
			}
			else 
				return( TRUE );
		}
	}

	/*
	 *	The item to be inserted should be located at the end of the list.
	 */

	if( dl_append( psave, phead ) != TRUE )
	{
		free( psave );
		return( FALSE );
	}
	else
		return( TRUE );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *	
 *	bool_typ dl_insert_before( phead, pcurrent, pitem )
 *	dl_head_typ *phead		-	Pointer to the head of the double linked list.
 *	dl_node_typ *pcurrent	-	Pointer to the current node.
 *	void *pitem				-	Item to be inserted.
 *
 *	DESCRIPTION
 *	This function inserts a node before the pcurrent node of a 
 *	double linked list.
 *
 *	RETURN
 *		TRUE	if the item is successfully inserted into the list.
 *		FALSE	if inserting the node fails.
 *
 */

bool_typ dl_insert_before(dl_head_typ *phead, dl_node_typ *pcurrent, void *pitem )
{
	dl_node_typ	*pleft;
	dl_node_typ *pnew;

	if( phead == NULL )
		return( FALSE );

	pleft = pcurrent->pprev;

	if( pleft == NULL )
		return( dl_insert( pitem, phead ) );
	
	if( (pnew = dl_inst_node( pitem, pleft, pcurrent )) != NULL  )
	{
		pcurrent->pprev = pnew;
		pleft->pnext = pnew;
		phead->length++;
		return( TRUE );
	}
	else
		return( FALSE );
}	

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *	
 *	bool_typ dl_insert_after( phead, pcurrent, pitem )
 *	dl_head_typ *phead		-	Pointer to the head of the double linked list.
 *	dl_node_typ *pcurrent	-	Pointer to the current node.
 *	void *pitem				-	Item to be inserted.
 *
 *	DESCRIPTION
 *	This function inserts a node after the pcurrent node of a 
 *	double linked list.
 *
 *	RETURN
 *		TRUE	if the item is successfully inserted into the list.
 *		FALSE	if inserting the node fails.
 *
 */

bool_typ dl_insert_after( phead, pcurrent, pitem )
register dl_head_typ *phead;
register dl_node_typ *pcurrent;
void *pitem;
{
	dl_node_typ	*pright;
	dl_node_typ *pnew;

	if( phead == NULL )
		return( FALSE );

	pright = pcurrent->pnext;

	if( pright == NULL )
		return( dl_append( pitem, phead ) );
	
	if( (pnew = dl_inst_node( pitem, pcurrent, pright )) != NULL  )
	{
		pcurrent->pnext = pnew;
		pright->pprev = pnew;
		phead->length++;
		return( TRUE );
	}
	else
		return( FALSE );
}	

/**	SYNOPSIS
 *
 *	#include <sys_list.h>
 *
 *	int dl_length( dl_head_typ *phead );
 *	dl_head_typ *phead	-	Pointer to the list.
 *
 *	DESCRIPTION
 *	Get the length of the list, given the head.
 *
 *	RETURN
 *		ERROR		-Pointer to head is NULL.
 *		non-ERROR	-Number of items in list.
 *
 */

int dl_length( phead )
dl_head_typ *phead;
{
    if( phead == NULL )
        return( ERROR );
    else
        return( phead->length );
}

bool_typ dl_add_dup( dl_head_typ *phead, void *pdata, 
	unsigned size, bool_typ front )
{
	void *pnew;
	bool_typ status;

	if( (pnew = (void *) MALLOC( size )) == NULL )
		return( FALSE );

	bytecopy( size, pdata, pnew );

	if( front == TRUE )
		status = dl_insert( pnew, phead );
	else
		status = dl_append( pnew, phead );

	if( status != TRUE )
		FREE( pnew );

	return( status );
}
