/**\file
 *
 * Class PathHub: simplified datahub access from C++
 * Class PathHubArray: treat a sequence of consecutive variable IDs in
 *		the database as an array of the same type
 *			
 *
 * The clt (client) wrappers for the Cogent Datahub implement the API
 * used by PATH AVCS QNX4 programs. This class is a C++ API for the
 * same functionality, see the PATH AVCS Common Library Reference Manual.
 *
 * Briefly, this API creates an association between a structure of a
 * certain size and an integer ID that is used to update and read a
 * variable of this structure type. This structure is implemented 
 * underneath by the Cogent Datahub string type.
 *
 * Later it may be desirable to implement this API directly on top of
 * the Cogent Datahub, or in other environments with different thread
 * and inter process communication primitives.
 * Sue Dickey April 2004
 *
 * Updated to use the Linux port of the QNX4 database, not Cogent.
 * Sue Dickey September 2007
 *
 */
#include <sys_os.h>

extern "C" {
#include <local.h>
#include <db_clt.h>
#include <timing.h>
}

#include <path_datahub.h>

/** PathHub constructor attaches to datahub with specified domain 
 */
PathHub::PathHub(char *task, char *domain) 
{
	// NULL pointer as second parameter means not networked database
	pclt = clt_login(task, (char *) NULL, domain, COMM_PSX_XPORT);

	if (pclt == NULL) {
		printf("%s fails to log in to datahub %s\n", task, domain);
		exit(1);
	}
}

PathHub::~PathHub()
{
	clt_logout(pclt);
}

/** createHubVar must be called by at least one process before accessing 
 *  the variable; we will not make a failed create a fatal error, but
 *  assume it was created by someone else
 */ 
void PathHub::createHubVar(int var, int size)
{
	if (!clt_create(pclt, var, var, size)) {
		printf("Hub var %d create failed\n", var);
	}
}

/** readHubVar returns pointer to variable with ID var in pdb_data
 */
void PathHub::readHubVar(int var, db_data_typ *pdb_data)
{
	if (!clt_read(pclt, var, var, pdb_data)) {
		printf("Hub var %d read failed\n", var);
	}
}

/** updateHubVar place data pointed to by pvalue pointer into datahub;
 *  pvalue will be the address of a structure of the indicated size in bytes
 */  
void PathHub::updateHubVar(int var, int size, void *pvalue)
{
	if (!clt_update(pclt, var, var, size, pvalue))
		printf("Hub var %d: update failed\n", var);
}

/** When a trigger is set, a waitHub operation will return when the
 * value of the variable has changed.
 */
void PathHub::setTrigger(int var)
{
	if (clt_trig_set( pclt, var, var ) == FALSE) 
		printf("Hub var %d trigger set failed\n", var);
}

void PathHub::unsetTrigger(int var)
{
	if (clt_trig_unset( pclt, var, var ) == FALSE) 
		printf("Hub var %d trigger set failed\n", var);
}

/** isTrigger returns 1 if the message received from the Datahub is a
 *  trigger on the specified variable, 0 otherwise.
 */
int PathHub::isTrigger(int recv_type, int var)
{
	if (recv_type == DB_TIMER)
		return 0;
	return (DB_TRIG_VAR(&trig_info) == (unsigned int) var);
}

/** Waits until the process receives a message, using the Cogent
 *  IP_Receive interface and the task and message structures allocated
 *  at datahub login. Returns the receive type of the message.
 */
int PathHub::waitHub()
{
	return (clt_ipc_receive(pclt,&trig_info, sizeof(trig_info)));
}

PathHubArray::PathHubArray(PathHub *phub, int first, int length, int size) 
{
	int i;

	firstID = first;
	arrayLength = length;
	typeSize = size;
	hubPointer = phub; 

	for (i = 0; i < arrayLength; i++)
		phub->createHubVar(firstID + i, typeSize);
			
}

/** isInBounds: Check if array index is within bounds
 */
int PathHubArray::isInBounds(int index) 
{
	
	if (index < 0  || index > arrayLength-1) { 
		printf("%d out of bounds for hub array %d, size %d\n",
			index, firstID, arrayLength);
		return 0;
	}
	fflush(stdout);
	return 1;
}
/** readElem: Read element index+firstID from database
 *  pdata should be a pointer to the base type 
 */
void PathHubArray::readElem(int index, void *pdata)
{
	PathHub *phub = hubPointer;
	db_data_typ db_data;
	if (isInBounds(index)) 
		phub->readHubVar(index+firstID, &db_data);
	memcpy(pdata, &db_data.value.user[0], typeSize);
}
/** Write element to database at index + firstID
 */
void PathHubArray::updateElem(int index, void *pdata)
{
	PathHub *phub = hubPointer;
#ifdef DO_TRACE
	printf("updateElem: index %d, size %d, dbID %d\n", 
		index, typeSize, index+firstID);
#endif
	if (isInBounds(index))
		phub->updateHubVar(index+firstID, typeSize, pdata);
}

PathCircBuff::PathCircBuff(int in, int out,
		PathHub *phub, int first, int length, int size):
	PathHubArray(phub, first, length,  size)
{
	int zero = 0;

	inID = in;	// holds index for insertion into buffer
	outID = out;	// holds index for deletion from buffer
	PathHub *tmp_phub = PathHubArray::hubPointer;

	tmp_phub->createHubVar(in, sizeof(int));
	tmp_phub->updateHubVar(in, sizeof(int), &zero);

	tmp_phub->createHubVar(out, sizeof(int));
	tmp_phub->updateHubVar(out, sizeof(int), &zero);

}

//default ~PathCircBuff 

/* isEmpty: nothing to delete
 */
int PathCircBuff::isEmpty()
{
	int inval, outval;
	db_data_typ db_data;
	PathHub *phub = PathHubArray::hubPointer;
	phub->readHubVar(inID, &db_data);
	inval = *((int *) (&db_data.value.user[0]));
	phub->readHubVar(outID, &db_data);
	outval = *((int *) (&db_data.value.user[0]));
	return (inval == outval);
}

/** isFull: cannot do an insert
 * Note that when the buffer is full the array actually has one empty slot
 */
int PathCircBuff::isFull()
{
	int inval, outval;
	db_data_typ db_data;
	PathHub *phub = PathHubArray::hubPointer;
	int length = PathHubArray::arrayLength;
	phub->readHubVar(inID, &db_data);
	inval = *((int *) (&db_data.value.user[0]));
	phub->readHubVar(outID, &db_data);
	outval = *((int *) (&db_data.value.user[0]));
	return (outval ? inval == outval-1 : inval == length - 1);
}

/** insertItem: put item in array and update in index
 */
int PathCircBuff::insertItem(void *p)
{
	int inval;
	int full;

	db_data_typ db_data;
	PathHub *phub = PathHubArray::hubPointer;

	full = isFull();
	
	if (!full) {
		phub->readHubVar(inID, &db_data);
		inval = *((int *) (&db_data.value.user[0]));

		PathHubArray::updateElem(inval, p);

		inval++;
		if (inval == PathHubArray::arrayLength) 
			inval = 0;

		phub->updateHubVar(inID, sizeof(int), &inval);
	}
	return !full;
}
	
/** deleteItem: get item from array and update out index
 */
int PathCircBuff::deleteItem(void *p)
{
	int outval;
	int empty;
	db_data_typ db_data;
	PathHub *phub = PathHubArray::hubPointer;

	empty = isEmpty();

	if (!empty) {
		phub->readHubVar(outID, &db_data);
		outval = *((int *) (&db_data.value.user[0]));

		PathHubArray::readElem(outval, p);

		outval++;
		if (outval == PathHubArray::arrayLength) 
			outval = 0;

		phub->updateHubVar(outID, sizeof(int), &outval);
	}
	return !empty;
}
	
