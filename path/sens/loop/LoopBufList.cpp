#include "LoopBufList.h"
#include <iostream>
#include <math.h>


LoopBufList::LoopBufList()
{
	head = NULL;
 	length =0;
}

LoopBufList::~LoopBufList()
{
	DestroyList();
}

/*
bool LoopBufList::InitList()
{
	if (!(head=new LoopBufNode))
		return false;
	head->next = NULL;
	return true;
}
*/

bool LoopBufList::DestroyList()
{
	if (head == NULL)
		return false;
	LoopBufNode *pTemp = NULL;
	while (head!=NULL)
	{
		pTemp = head;
		head = head->next;
		delete pTemp;
		pTemp = NULL;
	}
	length=0;
	return true;
}



bool LoopBufList::IsEmpty()
{
	if (head!=NULL)
		return false;
	return true; 
}


int LoopBufList::GetLength()
{
	return length;
}

// position is from 0 to 255
bool LoopBufList::GetNode(int position,LoopBufNode** node)
{
	LoopBufNode *pTemp = NULL;
	int curPos = -1;
	pTemp = this->head;
	while(pTemp!=NULL)
	{
		curPos++;
		if (curPos == position)
			break;
		pTemp = pTemp->next;
	}
	if (curPos!=position)
		return false;
	*node = pTemp;
	return true;        
} 



bool LoopBufList::SetNodeData(int position, LOOPS_TYP loop_typ)
{ 
	LoopBufNode *pTemp = NULL;
        if (!GetNode(position,&pTemp))
		return false;
	pTemp->loop_info = loop_typ;
	return true;
}



bool LoopBufList::GetNodeData(int position,LOOPS_TYP &loop_typ)
{
	LoopBufNode *pTemp = NULL;
	if (!GetNode(position,&pTemp))
		return false;
	loop_typ = pTemp->loop_info;
	return true;
}

// index is from 0 to 255
int LoopBufList::LocateElem(double ts)
{
	LoopBufNode *pTemp = NULL;
	int curIndex = 0;
	
	pTemp = this->head;
        double minVal = 3;
	int minIndex = curIndex;
	while(pTemp!=NULL)
	{
		if(fabs(pTemp->loop_info.ts-ts)<minVal)
		{
			minVal=fabs(pTemp->loop_info.ts-ts);
			minIndex = curIndex;
		}
		pTemp=pTemp->next;
		curIndex++;
	}
       
        if (minVal == 3)
		return -1;
	return minIndex;
}

bool LoopBufList::InsertNode(LOOPS_TYP loop_typ)
{
        LoopBufNode *pTemp;
        if (length==MAX_LIST)
	{	
		if(!DeleteNode())
			return false;
	}
	LoopBufNode * newNode = new LoopBufNode;
	newNode->loop_info = loop_typ;
	if (length==0)
	{
                newNode->next = NULL; 
 		this->head = newNode;
		this->length++;
        }		
	else
	{
		if (!GetNode(length-1,&pTemp))
			return false;
		newNode->next = pTemp->next;
		pTemp->next = newNode;
        	this->length++;
        }
        return true;
}

//delete the first node
bool LoopBufList::DeleteNode()
{
 	LoopBufNode *pTemp;
	pTemp = this->head;
	head = pTemp->next;
	delete pTemp;
        this->length--;
	return true;
}       		
       	
	

void LoopBufList::PrintData(int position)
{
	//LoopBufNode* pTemp;
	if (position>=0)
	{
		LOOPS_TYP  loop_tmp;
        	GetNodeData(position,loop_tmp);	
       	        printf("timestamp is %lf\n",loop_tmp.ts);
	}
}

		
















			









 	
	





















		












