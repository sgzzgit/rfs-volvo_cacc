/**\file
 *
 * Class PathHub: simplified datahub access from C++
 * Class PathHubArray: treat a sequence of consecutive variable IDs in
 *		the database as an array of the same type
 *			
 *
 */

#include <sys_os.h>
#include <db_clt.h>

/** PathHub has basic methods for reading, updating and triggering
 * on database variables, of known variable id "var"
 */
class PathHub {
public: 
	// pclt->chid used to initialize timers
	// pclt->hmsg and pclt->msginfo used when non-datahub and
	// datahub messages are both waiting on waitHub 
	db_clt_typ *pclt;
	trig_info_typ trig_info;
	PathHub(char *task, char *domain); // constructor logs in 
	~PathHub();	// destructor logs out
	void createHubVar(int var, int size);   // index var, data size
	void readHubVar(int var, db_data_typ *pdb_data);
	void updateHubVar(int var, int size, void *pvalue);
	void setTrigger(int var);
	void unsetTrigger(int var);
	int isTrigger(int recv_type, int var);
	int waitHub();	// wait for triggers, pulses and timers
};

/** PathHubArray allows a set of consecutive database variable IDs
 *  to be used for variables all of the same type and accessed like
 *  an array.
 */

class PathHubArray {
public:
	int firstID;	/// Database variable ID of first element
	int arrayLength;/// Number of elements in the array
	int typeSize;   /// Size of the base type of the array
	PathHub *hubPointer;	/// Pointer to datahub class
	
	PathHubArray(PathHub *phub, int first, int length, int size); 
	// default ~PathHubArray
	int isInBounds(int index); 
	void readElem(int index, void *pdata);
	void updateElem(int index, void *pdata);
};

/** PathCircBuff allows a PathHubArray to be used as a circular buffer
 *  by different producer/consumer processes.
 */
class PathCircBuff: public PathHubArray {
public:
	int inID, outID;

	// constructor adds two database variables to keep track of
	// insertion and deletion indices to PathHubArray constructor
	PathCircBuff(int in, int out,
			PathHub *phub, int first, int length, int size);
	//default ~PathCircBuff 
	// isEmpty returns 1 if buffer is empty, 0 otherwise
	int isEmpty();
	// isFull returns 1 if buffer is full, 0 otherwise
	int isFull();
	// insertItem adds item, adjusts "in" value, returns 0 on failure
	int insertItem(void *pitem);
	// deleteItem removes item, adjusts "out" value, returns 0 on failure
	int deleteItem(void *pitem);
};
	
