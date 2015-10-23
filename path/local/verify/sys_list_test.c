
#ifdef TEST1

main()
{
	dl_head_typ *phead;
	dl_node_typ *pnode;
	int *pdata;
	int i;

	phead = dl_create();
	for( i = 0; i < 10; i++ )
	{
		pdata = malloc( sizeof( int ) );
		*pdata = i;
		if( dl_append( (void *) pdata, phead ) == FALSE )
		{
			fprintf( stderr, "list allocation failed\n");
			exit(1);
		}
	}

	/* Traverse backward	*/
	pnode = phead->plast;
	while( pnode != NULL )
	{
		pdata = (int *) pnode->pitem;
		printf( "%3d ", *pdata );
		pnode = pnode->pprev;
	}
	printf( "\n" );

	/* Traverse forward		*/
	pnode = phead->pfirst;
	while( pnode != NULL )
	{
		pdata = (int *) pnode->pitem;
		printf( "%3d ", *pdata );
		pnode = pnode->pnext;
	}
	printf( "\n" );

	/* Traverse backward, deleting every other	*/
	pnode = phead->plast;
	while( pnode != NULL )
	{
		pdata = dl_rm_node( phead, &pnode );
		free( (void *) pdata );
		pnode = pnode->pprev;
	}
	/* Traverse backward	*/
	pnode = phead->plast;
	while( pnode != NULL )
	{
		pdata = (int *) pnode->pitem;
		printf( "%3d ", *pdata );
		pnode = pnode->pprev;
	}
	printf( "\n" );
}

#endif

#ifdef TEST2

struct one_page_str
{
	long data[64];
};

main()
{
	dl_head_typ *phead;
	long *pdata; 
	long i; 

	phead = dl_create();
	for( i = 0; i < 2000000; i++ )
	{
#ifndef BOMB
		if( phead->length > 2000 )
		{
			pdata = dl_rm_first( phead );
			free( pdata );
		}
#endif
		pdata = (void *) malloc( sizeof( struct one_page_str ) );
		*pdata = i;
		if( dl_append( (void *) pdata, phead ) == FALSE )
		{
			fprintf( stderr, "list allocation failed %ld\n", i);
			sleep(0);
		}
		if( (i % 1000) == 0 )
			printf("%ld\n", i );
	}
}
#endif

#ifdef TEST3

struct one_page_str
{
	long data[64];
};

main()
{
	ll_head_typ *phead;
	long *pdata; 
	long i; 

	phead = ll_create();

	for( i = 0; i < 2000; i++ )
	{
		pdata = (void *) malloc( sizeof( struct one_page_str ) );
		*pdata = i;
		if( ll_append( (void *) pdata, phead ) == FALSE )
		{
			fprintf( stderr, "list allocation failed %ld\n", i);
			sleep(0);
		}
	}
	while( (pdata = ll_rm_last( phead )) != NULL )
		free( pdata );
	printf("len = %d\n", phead->length );
}
#endif

#ifdef TEST4

struct one_page_str
{
	long data[64];
};

main()
{
	dl_head_typ *phead;
	long *pdata; 
	long i; 

	phead = dl_create();
	for( i = 0; i < 2000000; i++ )
	{
#ifndef BOMB
		if( phead->length > 2000 )
		{
			pdata = dl_rm_last( phead );
			free( pdata );
		}
#endif
		pdata = (void *) malloc( sizeof( struct one_page_str ) );
		*pdata = i;
		if( dl_append( (void *) pdata, phead ) == FALSE )
		{
			fprintf( stderr, "list allocation failed %ld\n", i);
			sleep(0);
		}
		if( (i % 1000) == 0 )
			printf("%ld\n", i );
	}
}
#endif

#ifdef TEST5

struct test_str
{
	long data[384];
};

main()
{
	dl_head_typ *phead;
	struct test_str data; 
	long i; 

	phead = dl_create();
	for( i = 0; i < 200000; i++ )
	{
		if( dl_mv_node( (void *) &data, phead, 
			sizeof( struct test_str), 50 ) == FALSE )
		{
			fprintf( stderr, "dl_mv_node() failed %ld\n", i);
			sleep(0);
		}
		if( (i % 1000) == 0 )
			printf("%ld\n", i );
	}
}

#endif

#ifdef TEST_FIRST

#define KEY_MSG				"Hello"
#define LIST_ITERATIONS		1000	
#define NUM_MESSAGES		16

static char message[ NUM_MESSAGES ][ 6 ] =
{
	"H",
	"He",
	"Hel",
	"Hell",
	"Hello",
	"H",
	"He",
	"Hel",
	"Hell",
	"Hello",
	"H",
	"He",
	"Hel",
	"Hell",
	"Hello",
	"Hello"
};

/*
 *	DESCRIPTION
 *	This tests dl_first() and dl_next().
 *	It creates a double linked list, searches the list, and keeps
 *	track of how many times it finds an item.
 */
main()
{
	dl_head_typ *phead;
	dl_node_typ *pnode;
	dl_node_typ *pnode_next;
	int i;
	int first_cntr = 0, next_cntr = 0;	

	int compare_func();

	if( ( phead = dl_create() ) == NULL )
	{
		printf( "couldn't initialize the head \n");
		exit(0);
	}
	
	/*	Set up the linked list */
	for( i = 0; i < NUM_MESSAGES; i++ ) 
		dl_insert( message[i], phead );


	/*	Iterate over the list */

	printf("TESTING           :\tBeat on dl_first() and dl_next().\n");

	for( i = 0; i < LIST_ITERATIONS; i++ )
	{
		if( ( pnode = dl_first( phead, (void *)KEY_MSG, 
			compare_func ) ) == NULL )
		{
			printf( "couldn't find the string\n");
			exit(0);
		}
		else
			first_cntr++;

		while( ( pnode_next = dl_next( pnode->pnext, (void *)KEY_MSG, 
			compare_func ) ) != NULL )
		{
			pnode = pnode_next;
			next_cntr++;
		}
	}

	/*	
	 *	Error Results.  Should expect next_cntr to be three times the
	 *	size of the first_cntr.
	 */
	printf( "Results First     :\t%d\t(expect 1000)\n", first_cntr );
	printf( "Results Next      :\t%d\t(expect 3000)\n\n", next_cntr );
	
	printf("Testing dl_first():\tNULL condition\n");
	if( ( pnode = dl_first( NULL, (void *)KEY_MSG, compare_func ) ) == NULL )
	{
		printf( "dl_first()        :\tERROR in calling parameters.\n\n");
	}

	printf("Testing dl_next() :\tNULL condition\n");
	if( ( pnode = dl_first( NULL, (void *)KEY_MSG, compare_func ) ) == NULL )
	{
		printf( "dl_next()         :\tERROR in calling parameters.\n\n");
	}


	/*	Free the resources */
	printf("End Testing.  Free resources.\n");
	dl_free( phead );
	exit(0);
}

int compare_func( pkey, pitem )
void *pkey;
void *pitem;
{
	return( strcmp( (char *)pkey, (char *)pitem ) );
}

#endif




#ifdef TEST_LL_FIRST

#define KEY_MSG				"Hello"
#define LIST_ITERATIONS		1000	
#define NUM_MESSAGES		16

static char message[ NUM_MESSAGES ][ 6 ] =
{
	"H",
	"He",
	"Hel",
	"Hell",
	"Hello",
	"H",
	"He",
	"Hel",
	"Hell",
	"Hello",
	"H",
	"He",
	"Hel",
	"Hell",
	"Hello",
	"Hello"
};

/*
 *	DESCRIPTION
 *	This tests ll_first() and ll_next().
 *	It creates a single linked list, searches the list, and keeps
 *	track of how many times it finds an item.
 */
main()
{
	ll_head_typ *phead;
	ll_node_typ *pnode;
	ll_node_typ *pnode_next;
	int i;
	int first_cntr = 0, next_cntr = 0;	

	int compare_func();

	if( ( phead = ll_create() ) == NULL )
	{
		printf( "couldn't initialize the head \n");
		exit(0);
	}
	
	/*	Set up the linked list */
	for( i = 0; i < NUM_MESSAGES; i++ ) 
		ll_insert( message[i], phead );


	/*	Iterate over the list */

	printf("TESTING           :\tBeat on ll_first() and ll_next().\n");

	for( i = 0; i < LIST_ITERATIONS; i++ )
	{
		if( ( pnode = ll_first( phead, (void *)KEY_MSG, 
			compare_func ) ) == NULL )
		{
			printf( "couldn't find the string\n");
			exit(0);
		}
		else
			first_cntr++;

		while( ( pnode_next = ll_next( pnode->pnext, (void *)KEY_MSG, 
			compare_func ) ) != NULL )
		{
			pnode = pnode_next;
			next_cntr++;
		}
	}

	/*	
	 *	Error Results.  Should expect next_cntr to be three times the
	 *	size of the first_cntr.
	 */
	printf( "Results First     :\t%d\t(expect 1000)\n", first_cntr );
	printf( "Results Next      :\t%d\t(expect 3000)\n\n", next_cntr );
	
	printf("Testing ll_first():\tNULL condition\n");
	if( ( pnode = ll_first( NULL, (void *)KEY_MSG, compare_func ) ) == NULL )
	{
		printf( "ll_first()        :\tERROR in calling parameters.\n\n");
	}

	printf("Testing ll_next() :\tNULL condition\n");
	if( ( pnode = ll_first( NULL, (void *)KEY_MSG, compare_func ) ) == NULL )
	{
		printf( "ll_next()         :\tERROR in calling parameters.\n\n");
	}


	/*	Free the resources */
	printf("End Testing.  Free resources.\n");
	ll_free( phead );
	exit(0);
}

int compare_func( pkey, pitem )
void *pkey;
void *pitem;
{
	return( strcmp( (char *)pkey, (char *)pitem ) );
}

#endif



#ifdef TEST_PQ_ADD

struct data_str
{
	int priority;
	char identifier[20];
};

struct data_str queue_data[] =
{
	 5,	"five",
	 7,	"seven",
	 3,	"three",
	 1,	"one",
	 9,	"nine",
	 6,	"1st",
	 0,	"zero",
	 8,	"eight",
	 6,	"2nd",
	 2,	"two",
	10,	"ten",
	 4,	"four",
	 6,	"3rd"
};


/*
 *	DESCRIPTION
 *	This tests dl_pq_add().
 *	It creates a double linked list, inserts nodes into the list in 
 *	descending order, and prints the list items.
 *
 */
main()
{
	dl_head_typ *phead;
	dl_node_typ *pnode;
	struct data_str *pdata;
	struct data_str entry;
	int i;

	int compare_func();

	if( ( phead = dl_create() ) == NULL )
	{
		fprintf( stderr, "couldn't initialize the head \n" );
		exit(EXIT_FAIL);
	}

	/*	Set up the priority queue */
	for( i = 0; i < (sizeof( queue_data) / sizeof( struct data_str )); i++ )  
	{
		entry.priority = queue_data[i].priority;
		sprintf( entry.identifier, "%s", queue_data[i].identifier );

		if( ( dl_pq_add( phead, (void *) &entry, compare_func, 
					sizeof( struct data_str ) ) == FALSE ) ) 
		{	
			fprintf( stderr, "error in dl_pq_add()\n" );
			exit(EXIT_FAIL);
		}	
	}

	/* Traverse backward */
	printf("\nThe list in backward direction:\n");
	for( pnode = phead->plast; pnode != NULL; pnode=pnode->pprev )
	{
		pdata = (struct data_str *) pnode->pitem;
		printf( "%4d  %s\n", pdata->priority, pdata->identifier );
	}

	/* Traverse forward */
	printf("\nThe list in forward direction:\n");
	for( pnode = phead->pfirst; pnode != NULL; pnode=pnode->pnext )
	{
		pdata = (struct data_str *) pnode->pitem;
		printf( "%4d  %s\n", pdata->priority, pdata->identifier );
	}

	/*	Free the resources	*/
	printf( "\nEnd Testing.  Free resources.\n" );
	dl_free( phead );
	exit( EXIT_SUCCESS );
}


/*
 *	This is a user supplied function for dl_pq_add().
 *	This function compares two given data. 
 *	The first data is a member of an item pkey,
 *	the second data is a member of an item pitem.
 *
 *	RETURN
 *		< 0		if item in pkey less than item in pitem
 *		  0		if item in pkey equal to item in pitem
 *		> 0		if item in pkey greater than item in pitem
 *
 */
int compare_func( pkey, pitem )
void *pkey;
void *pitem;
{
	struct data_str *pkey_data;
	struct data_str *pitem_data;

	pkey_data  = (struct data_str *) pkey;
	pitem_data = (struct data_str *) pitem;
	return( pkey_data->priority - pitem_data->priority );
}

#endif



#ifdef  TEST_INSERT_BEFORE

#define MAX_NO	9

/*
 *	DESCRIPTION
 *	This tests dl_insert_before().
 *	It creates a double linked list, continuly inserts a node in front 
 *	of the last item of the list for MAX_NO of times, and prints the 
 *	list items.
 *
 */
main()
{
	dl_head_typ *phead;
	dl_node_typ *pnode;
	int *pdata;
	int i;

	if( ( phead = dl_create() ) == NULL )
	{
		fprintf( stderr, "couldn't initialize the head \n" );
		exit( EXIT_FAIL );
	}

	pdata = (int *) malloc( sizeof( int ) );
	*pdata = 9;
	if( dl_append( (void *) pdata, phead ) == FALSE )
	{
		fprintf( stderr, "list allocation failed\n");
		exit( EXIT_FAIL );
	}

	pnode = phead->plast; 
	printf( "\nThe last item in the list is %d\n\n", *pdata );
	pdata = (int *)pnode->pitem;

	printf( "Inserting an item in front of the last item of the list.\n" );
	for( i = 1; i < MAX_NO; i++ )
	{
		pdata = (int *) malloc( sizeof( int ) );
		*pdata = i;

		if( dl_insert_before( phead, phead->plast, (void *)pdata ) == FALSE )
		{
			fprintf( stderr, "error in dl_insert_before()\n" );
			exit( EXIT_FAIL );
		}

		for( pnode = phead->pfirst; pnode != NULL; pnode=pnode->pnext )
		{
			pdata = (int *)pnode->pitem;
			printf( "%4d", *pdata );
		}
		printf("\n");
	} 

	/* Traverse backward */
	printf("\nThe list in backward direction:\n");
	for( pnode = phead->plast; pnode != NULL; pnode=pnode->pprev )
	{
		pdata = (int *)pnode->pitem;
		printf( "%4d", *pdata );
	}

	/* Traverse forward */
	printf("\n\nThe list in forward direction:\n");
	for( pnode = phead->pfirst; pnode != NULL; pnode=pnode->pnext )
	{
		pdata = (int *)pnode->pitem;
		printf( "%4d", *pdata );
	}

	/*	Free the resources	*/
	printf( "\n\nEnd Testing.  Free resources.\n" );
	dl_free( phead );
	exit( EXIT_SUCCESS );
}

#endif



#ifdef  TEST_INSERT_AFTER

#define MAX_NO	9

/*
 *	DESCRIPTION
 *	This tests dl_insert_after().
 *	It creates a double linked list, continuly inserts a node behind 
 *	the first item of the list for MAX_NO of times, and prints the 
 *	list items.
 *
 */
main()
{
	dl_head_typ *phead;
	dl_node_typ *pnode;
	int *pdata;
	int i;

	if( ( phead = dl_create() ) == NULL )
	{
		fprintf( stderr, "couldn't initialize the head \n" );
		exit( EXIT_FAIL );
	}

	pdata = (int *) malloc( sizeof( int ) );
	*pdata = 9;
	if( dl_append( (void *) pdata, phead ) == FALSE )
	{
		fprintf( stderr, "list allocation failed\n");
		exit( EXIT_FAIL );
	}

	pnode = phead->pfirst; 
	printf( "\nThe first item in the list is %d\n\n", *pdata );
	pdata = (int *)pnode->pitem;

	printf( "Inserting an item behind the first item of the list.\n" );
	for( i = 1; i < MAX_NO; i++ )
	{
		pdata = (int *) malloc( sizeof( int ) );
		*pdata = i;

		if( dl_insert_after( phead, phead->pfirst, (void *)pdata ) == FALSE )
		{
			fprintf( stderr, "error in dl_insert_after\n" );
			exit( EXIT_FAIL );
		}

		for( pnode = phead->pfirst; pnode != NULL; pnode=pnode->pnext )
		{
			pdata = (int *)pnode->pitem;
			printf( "%4d", *pdata );
		}
		printf("\n");
	} 

	/* Traverse backward */
	printf("\nThe list in backward direction:\n");
	for( pnode = phead->plast; pnode != NULL; pnode=pnode->pprev )
	{
		pdata = (int *)pnode->pitem;
		printf( "%4d", *pdata );
	}

	/* Traverse forward */
	printf("\n\nThe list in forward direction:\n");
	for( pnode = phead->pfirst; pnode != NULL; pnode=pnode->pnext )
	{
		pdata = (int *)pnode->pitem;
		printf( "%4d", *pdata );
	}

	/*	Free the resources	*/
	printf( "\n\nEnd Testing.  Free resources.\n" );
	dl_free( phead );
	exit( EXIT_SUCCESS );
}

#endif


