#include "loop.h"
#define MAX_LIST 256

typedef struct LoopBufNode 
{
        LOOPS_TYP loop_info;
	struct LoopBufNode *next;
};

class LoopBufList
{
private:
     	LoopBufNode *head;
	int length;
public:
	LoopBufList();
	~LoopBufList();
	//bool InitList();
	bool DestroyList();
	bool IsEmpty();
	int GetLength();
        bool GetNode(int position, LoopBufNode** node);
        bool SetNodeData(int position, LOOPS_TYP loop_typ);
        bool GetNodeData(int position,LOOPS_TYP &loop_typ);
	int LocateElem(double ts);
	bool InsertNode(LOOPS_TYP loop_typ );
	bool DeleteNode();
        void PrintData(int position);
};				



