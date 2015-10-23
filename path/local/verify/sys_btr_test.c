
#ifdef TEST

#include <sys_time.h>
#include <sys_list.h>
#include <getopt.h>

#define DEFAULT_COUNT		50
#define DEFAULT_NAME		"id.db"

#define IRM_ID_LEN			17

extern double drand48();
extern char *index();

void print_node();
static int id_cmp();

struct node_str 
{
	char id[IRM_ID_LEN+1];
	char data[MAX_LINE_LEN+1];
};

struct node_str bad_node =
{
	"Invalid", "Data"
}; 

/*	Test driver.
 *	Run with the following data in a file, the tree should perform:
 *		single rotation to the left at root,
 *		double rotation to the right (non-root)
 *		double rotation to the left (non-root)
 *		single rotation to the right (non-root)

	Test data:

	d node 1
	e node 2
	k node 3 single, at root
	f node 4
	j node 5 double, nonroot
	c node 6
	i node 7
	g node 8 double, nonroot
	b node 9 single, nonroot


 *	Run with the following data in a file, the tree should perform:
 *		double rotation to the right at root,

	Test data:
	c node 1
	a node 2
	b node 3 double, root
 */

main( argc, argv )
int argc;
char *argv[];
{
	int load_data();

	bool_typ verbose;
	int option;
	void *found;
	dl_head_typ *phead;
	FILE *pfin;
	int length, i, find;
	int num_nodes;
	dl_node_typ *plnode;
	btr_node_typ *proot;
	char *pname;
	int count;

	pname = DEFAULT_NAME;
	count = DEFAULT_COUNT;
	verbose = FALSE;

	while( ( option = getopt( argc, argv, "f:n:v" )) != EOF )
	{
		switch( option )
		{
		case 'f':
			pname = optarg;
			break;

		case 'n':
			count = atoi( optarg );
			break;

		case 'v':
			verbose = TRUE;
			break;

		default:	
			exit(1);
		}
	}

	if( (pfin = fopen( pname, "r" )) == NULL )
		error_exit( "Can't open input file.\n" );

	if( (phead = dl_create()) == NULL )
		error_exit( "Can't create list.\n" );

	if( (proot = btr_inst_node( NULL )) == NULL )
		error_exit( "Can't create tree root.\n" );

	num_nodes = load_data( pfin, &proot, phead );
	printf( "%ld\n", get_sys_time() );
	for( i = 0; i < count*num_nodes; i++ )
	{
		find = drand48()*num_nodes;
		if( (plnode = dl_nth_node( find, phead )) != NULL )
		{
			if( (found =
				btr_find( (void *) plnode->pitem, &proot, id_cmp )) == NULL )
			{
				printf( "Item not found.\n" );
			}
		}
	}
	printf( "%ld\n", get_sys_time() );
	if( verbose == TRUE )
		btr_dump( proot, print_node );
	fclose( pfin );
}

int load_data( pfin, proot, phead )
FILE *pfin;
btr_node_typ **proot;
dl_head_typ *phead;
{
	int n;
	void *found;
	char buffer[MAX_LINE_LEN+1];
	char *ps;
	struct node_str *ptnode, *plnode;

	/* locate nodes into the tree */

	n = 0;
	while( readline( pfin, buffer, MAX_LINE_LEN ) != ERROR )
	{
		if( ((ptnode = (struct node_str *) 
			malloc( sizeof( struct node_str ))) == NULL)
		|| ((plnode = (struct node_str *) 
			malloc( sizeof( struct node_str ))) == NULL) )
		{
			error_exit( "malloc() failure.\n" );
		}

		ps = index( buffer, ' ' );
		*ps++ = END_OF_STRING;
		strcpy( ptnode->id, buffer );
		strcpy( ptnode->data, ps );
		bytecopy( sizeof( struct node_str ), ptnode, plnode );
		found = btr_avl( ptnode, proot, id_cmp );
		dl_append( plnode, phead );
		if( found != ptnode )
			printf( "Duplicate node ID.\n" );
		else
			n++;

/*
		btr_dump( *proot, print_node );
*/
	}

/*	
	dl_append( &bad_node, phead ); 
*/
	return( ++n );
}

/* 
 *	Print out nodes in alphabetical order 
 */

void print_node( pnode, order, level )
struct node_str *pnode;
btr_visit_typ order;
int level;
{
	if( (order == BTR_INORDER) || (order == BTR_LEAF) )
		printf("%-5s %s\n", pnode->id, pnode->data );
}

static int id_cmp( pa, pb )
struct node_str *pa, *pb;
{
	return( strcmp( pa->id, pb->id ) );
}

#endif
