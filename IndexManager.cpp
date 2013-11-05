#define _CRT_SECURE_NO_WARNINGS
#include "ClassDefinition.h"
#include "IndexManager.h"
#include "BufferManager.h"
extern BufferManager buff;
//=================Node========================
//OK
int Node::getPointer(void* block,int start){
	int p=0;
	char* ptr = static_cast<char*>(block);
	ptr += start;
	for(int i = 0; i < POINTERSIZE; i++)
	{
		p = p*10 + *ptr - '0';
		ptr ++;
	}
	return p;
}
//OK
int Node::getNumber(void* block, int start){
	int n = 0;
	char * ptr = static_cast<char*>(block);
	ptr += start;
	for(int i = 0; i < NUMBERSIZE; i++)
	{
		n = n*10 + *ptr - '0';
		ptr ++ ;
	}
	return n;
}
//=================Branch=====================
//OK
Branch::Branch(void* block, bool blank){
	if(!blank)
	{
		char* p = static_cast<char*>(block);
		isRoot = (*p == '1');
		isLeaf = false;
		int iNum = getNumber(block,2);
		ptrFather = getPointer(block,6);
		attrLength = getNumber(block,11);
		itemCount = 0;
		ptrBlock = block;
		//fill in itemList
		p += 15;
		for(int i = 0; i < iNum; i++)
		{
			string s = "";
			for(int j = 0; j < attrLength; j++)
			{
				s += *p;
				p++;
			}
			int pointer = getPointer(block, (p - (char*)block));//may error
			p += POINTERSIZE;
			BranchItem item(s,pointer);
			InsertInBranch(item);
		}
	}
	else//make a blank branch
	{
		isRoot = false;
		isLeaf = false;
		itemCount = 0;
		ptrBlock = block;
	}
}
//OK
void Branch::InsertInBranch(BranchItem item)
{
	itemCount++;
	list<BranchItem>::iterator i;
	if(itemList.size() == 0)
		itemList.push_front(item);
	else{
		for(i = itemList.begin();i != itemList.end(); i++)
		{
			if((*i).key > item.key) break;
		}
		if(i != itemList.end())
			itemList.insert(i,item);
		else
			itemList.push_back(item);
	}
}
//OK
void Branch:: DeleteFromBranch(string key){
	list<BranchItem>::iterator i;
	if(itemList.size() == 0);
	else{
		for(i = itemList.begin(); i != itemList.end(); i++)
		{
			if(i->key == key)
			{
				itemList.erase(i);
				itemCount--;
				return;
			}
		}
	}
}
//OK
Branch::~Branch(){
	//change value in memory block & write back to file
	printf("~Branch() called.\n");
	char* p = static_cast<char*>(ptrBlock);
	*p = '0' + (isRoot == true);
	p++;
	*p = '0' + (isLeaf == true);
	p++;
	char icount[4];
	_itoa(itemCount,icount,10);
	std::string trans(icount);
	while(trans.size()<NUMBERSIZE)
		trans = '0' + trans;
	strncpy(p,trans.c_str(),NUMBERSIZE);
	p += NUMBERSIZE;
	
	char ptrs[5];
	_itoa(ptrFather,ptrs,10);
	trans = ptrs;
	while(trans.size()<POINTERSIZE)
		trans = '0' + trans;
	strncpy(p,trans.c_str(),POINTERSIZE);
	p += POINTERSIZE;

	_itoa(attrLength,icount,10);
	trans = icount;
	while(trans.size()<NUMBERSIZE)
		trans = '0' + trans;
	strncpy(p,trans.c_str(),NUMBERSIZE);
	p += NUMBERSIZE;

	list<BranchItem>::iterator i;
	for(i = itemList.begin(); i != itemList.end(); i++)
	{
		string key = (*i).key;
		int pi = (*i).ptr;
		strncpy(p,key.c_str(),attrLength);
		p += attrLength;

		_itoa(pi,ptrs,10);
		trans = ptrs;
		while(trans.size() < POINTERSIZE)
			trans = '0' + trans;
		strncpy(p,trans.c_str(),POINTERSIZE);
		p += POINTERSIZE;
	}
	buff.writeBack(ptrBlock);
	buff.unlock(ptrBlock);
}
//=================Leaf========================
//OK
Leaf::Leaf(void* block, bool blank){
	if(!blank)
	{
		char* p = static_cast<char*>(block);
		isRoot = (*p == '1');
		isLeaf = true;
		int itNum = getNumber(block,2);
		itemCount = 0;
		ptrFather = getPointer(block,6);
		attrLength = getNumber(block,11);
		leftSibling = getPointer(block,15);
		rightSibling = getPointer(block,20);
		ptrBlock = block;
		//fill in itemList
		p += 25;
		for(int i = 0; i < itNum; i++)
		{
			string s = "";
			for(int j = 0; j < attrLength; j++)
			{
				s += *p;
				p++;
			}
			int bno = getNumber(block,(p-(char*)block));
			p += NUMBERSIZE;
			int off = getNumber(block,(p-(char*)block));
			p += NUMBERSIZE;
			LeafItem item(s,bno,off);
			InsertInLeaf(item);
		}
	}
	//make a blank leaf
	else
	{
		isRoot = false;
		isLeaf = true;
		itemCount = 0;
		ptrBlock = block;
	}
}
//OK
void Leaf::InsertInLeaf(LeafItem item){
	itemCount++;
	list<LeafItem>::iterator i;
	if(itemList.size() == 0)
		itemList.push_front(item);
	else{
		for(i = itemList.begin();i != itemList.end(); i++)
		{
			if((*i).key > item.key) break;
		}
		if(i != itemList.end())
			itemList.insert(i,item);
		else
			itemList.push_back(item);
	}
}
//OK
void Leaf::DeleteFromLeaf(string key){
	list<LeafItem>::iterator i;
	if(itemList.size() == 0);
	else
	{
		for(i = itemList.begin(); i != itemList.end(); i++)
		{
			if(i->key == key)
			{
				itemList.erase(i);
				itemCount--;
				return;
			}
		}
	}
}
//OK
Leaf::~Leaf(){
	//change value in memory block & write to file
	printf("~Leaf() called.\n");
	char* p = static_cast<char*>(ptrBlock);
	*p = '0' + (isRoot == true);
	p++;
	*p = '0' + (isLeaf == true);
	p++;
	char icount[4];
	_itoa(itemCount,icount,10);
	std::string trans(icount);
	while(trans.size() < NUMBERSIZE)
		trans = '0' + trans;
	strncpy(p,trans.c_str(),NUMBERSIZE);
	p += NUMBERSIZE;

	char ptrs[5];
	_itoa(ptrFather,ptrs,10);
	trans = ptrs;
	while(trans.size() < POINTERSIZE)
		trans = '0' + trans;
	strncpy(p,trans.c_str(),POINTERSIZE);
	p += POINTERSIZE;
	
	_itoa(attrLength,icount,10);
	trans = icount;
	while(trans.size() < NUMBERSIZE)
		trans = '0' + trans;
	strncpy(p,trans.c_str(),NUMBERSIZE);
	p += NUMBERSIZE;
	
	_itoa(leftSibling,ptrs,10);
	trans = ptrs;
	while(trans.size() < POINTERSIZE)
		trans = '0' + trans;
	strncpy(p,trans.c_str(),POINTERSIZE);
	p += POINTERSIZE;

	_itoa(rightSibling,ptrs,10);
	trans = ptrs;
	while(trans.size() < POINTERSIZE)
		trans = '0' + trans;
	strncpy(p,trans.c_str(),POINTERSIZE);
	p += POINTERSIZE;

	list<LeafItem>::iterator i;
	for(i = itemList.begin(); i != itemList.end(); i++)
	{
		string key = (*i).key;
		int bno = (*i).blockNo;
		int off = (*i).offset;

		strncpy(p,key.c_str(),attrLength);
		p += attrLength;
		
		_itoa(bno,icount,10);
		trans = icount;
		while(trans.size() < NUMBERSIZE)
			trans = '0' + trans;
		strncpy(p,trans.c_str(),NUMBERSIZE);
		p += NUMBERSIZE;

		_itoa(off,icount,10);
		trans = icount;
		while(trans.size() < NUMBERSIZE)
			trans = '0' + trans;
		strncpy(p,trans.c_str(),NUMBERSIZE);
		p += NUMBERSIZE;
	}
	buff.writeBack(ptrBlock);
	buff.unlock(ptrBlock);
}
//=================IndexManager================
/*
void IndexManager::CreateIndex(const Table& tableInfo, Index& indexInfo){
	//1 : create a new folder for a table
	string attrName;
	string path = "\Index\\" + indexInfo.table + '\\' + indexInfo.name;
	mkdir(path.c_str());
	//2.create root
	char no[4];
	Node::nodeCount = 0;
	itoa(Node::nodeCount,no,10);
	string fileName = no;
	fileName += ".NODE";
	void* p = buff.read(path+'\\'+fileName);
	Leaf root(p,true);//blank leaf
	root.isRoot = true;
	root.ptrFather = Node::nodeCount;//point to self
	root.attrLength = indexInfo.length;
	root.leftSibling = Node::nodeCount;
	root.rightSibling = Node::nodeCount;
	//root.itemList
	indexInfo.nodeCount ++;

	//---get data from table and insert in B+ tree
	string tableFile;
	char num[10];
	for(int j = 0; j < tableInfo.blockCount; j++)
	{
		tableFile = tableInfo.name + itoa(j,num,10) + ".TABLE";
		void* p = buff.read(tableFile);
		//?? : add recordCount for each table ? or each block of a table ?(assume there is recordCount for each block)
		for(int k = 0; k < (*q).recordCount; k++)
		{
			string key;
			int offset;
			//TODO : get a tuple and parse it
			//TODO : assemble a LeafItem
			LeafItem item(key,j,offset);
			InsertValue(indexInfo,item);
		}
		buff.unlock(tableFile);
	}
}
*/
/*
void IndexManager::DropIndex(Index& indexInfo){
	string path = "\Index\\" + indexInfo.table + '\\' + indexInfo.name;
	string fileName;
	char num[4];
		
	for(int i = 0; i < indexInfo.nodeCount; i++)
	{
		fileName = indexInfo.name + "_" + itoa(i,num,10) + ".NODE";
		remove(fileName.c_str());
	}
}*/
/*
Result IndexManager::EqualQuery(const Table& tableInfo,const Index& indexInfo, string key, int nodeNo=1){
	char no[4];
	Result result;
	//read root
	string path = "\Index\\"+indexInfo.table + '\\'+ indexInfo.name + '\\';
	string fileName = itoa(nodeNo,no,10);
	fileName += ".NODE";

	void* p = buff.read(fileName);
	char* pv = static_cast<char*>(p);
	//reach leaf
	if(*(pv+1) == '1')
	{
		Leaf node(p,false);//make a nonempty leaf
		list<LeafItem>::iterator i;
		for(i = node.itemList.begin(); i != node.itemList.end(); i++)
		{
			//get data from table
			if((*i).key == key)
			{
				fileName = indexInfo.table + ".TABLE";
				//TODO : assemble result
				return result;
			}
		}
	}
	//not reach leaf, go down
	else
	{
		Branch node(p,false);//make a nonempty branch
		list<BranchItem>::iterator i;
		for(i = node.itemList.begin(); i != node.itemList.end(); i++)
		{
			if((*i).key > key)
			{
				if(i == node.itemList.begin())
					return result;
				else
				{
					i--;
					break;
				}
			}
		}
		if(i == node.itemList.end()) i--;
		result = EqualQuery(tableInfo,indexInfo,key,(*i).ptr);
	}
	return result;
}
*/
/*
BranchItem  IndexManager::InsertValue(Index& indexInfo,LeafItem item, int nodeNo=1){
	char no[4];
	//read root
	string path = "\Index\\"+indexInfo.table + '\\'+ indexInfo.name + '\\';
	string fileName = itoa(nodeNo,no,10);
	fileName += ".NODE";
	void* p = buff.read(fileName);
	char* pv = static_cast<char*>(p);

	BranchItem breturn;//empty item

	//node is leaf
	if(*(pv+1) == '1'){
		Leaf node(p,false);//make a nonempty leaf
		node.InsertInLeaf(item);
		//if node is full after insertion, split
		if(node.itemList.size() > indexInfo.maxItems)
		{
			//node is root : add 1 leaf and 1 branch (2 branchitem)
			if(node.isRoot)
			{
				indexInfo.nodeCount += 2;
				//make new leaf(blank)
				fileName = itoa(Node::nodeCount+1,no,10);
				fileName += ".NODE";
				p = buff.read(fileName);
				Leaf newleaf(p,true);
				//make new branch(blank)
				fileName = itoa(Node::nodeCount+1,no,10);
				fileName += ".NODE";
				p = buff.read(fileName);
				Branch newroot(p,true);
				//adjust leaves
				while(newleaf.itemCount < node.itemCount)
				{
					LeafItem item = node.itemList.back();
					newleaf.InsertInLeaf(item);
					node.itemCount--;
					node.itemList.pop_back();
				}
				newleaf.attrLength = indexInfo.length;
				newleaf.leftSibling = nodeNo;
				newleaf.ptrFather = Node::nodeCount;//after making new root
				newleaf.rightSibling = node.rightSibling;
				node.rightSibling = Node::nodeCount-1;
				//make new root
				newroot.attrLength = indexInfo.length;
				BranchItem bitem;
				list<LeafItem>::iterator i;
				i = node.itemList.begin();
				bitem.key = (*i).key;
				bitem.ptr = nodeNo;
				newroot.InsertInBranch(bitem);
				i = newleaf.itemList.begin();
				bitem.key = (*i).key;
				bitem.ptr = Node::nodeCount-1;
				newroot.InsertInBranch(bitem);
				return breturn;
			}
			//node is not root : cascade, add 1 leaf and 1 branchitem
			else{
				indexInfo.nodeCount += 1;
				//make new leaf(blank)
				fileName = itoa(Node::nodeCount+1,no,10);
				fileName += ".NODE";
				p = buff.read(fileName);
				Leaf newleaf(p,true);
				while(newleaf.itemCount < node.itemCount)
				{
					LeafItem item = node.itemList.back();
					newleaf.InsertInLeaf(item);
					node.itemCount--;
					node.itemList.pop_back();
				}
				newleaf.attrLength = indexInfo.length;
				newleaf.leftSibling = nodeNo;
				newleaf.ptrFather = node.ptrFather;//after making new root
				newleaf.rightSibling = node.rightSibling;
				node.rightSibling = Node::nodeCount;
				//TODO :update ptr to node 
				list<LeafItem>::iterator i = newleaf.itemList.begin();
				breturn.key = (*i).key;
				breturn.ptr = Node::nodeCount;
				return breturn;
			}
		}
		//node is not full after insertion
		else{
			return breturn;
		}
	}
	//node is branch
	else{
			Branch node(p,false);//make a nonempty branch
			list<BranchItem>::iterator i = node.itemList.begin();
			if(item.key < (*i).key)//smaller than the smallest
				(*i).key = item.key;//update so that the item can go down
			for(i = node.itemList.begin(); i != node.itemList.end(); i++)
			{
				if((*i).key>item.key)
					break;
			}
			i--;
			BranchItem ret = InsertValue(indexInfo,item,(*i).ptr);
				
			//split doesn't occur
			if(ret.key == "")
			{
				return breturn;	
			}
			//split occurs
			else{
				node.InsertInBranch(ret);
				//node is full after insertion
				if(node.itemCount > indexInfo.maxItems)
				{
					//node is root : add 2 branch
					if(node.isRoot)
					{
						indexInfo.nodeCount += 2;
						fileName = itoa(Node::nodeCount+1,no,10);
						fileName += ".NODE";
						p = buff.read(fileName);
						Branch newbranch(p,true);
						fileName = itoa(Node::nodeCount+1,no,10);
						fileName += ".NODE";
						p = buff.read(fileName);
						Branch newroot(p,true);
						//newbranch
						while(newbranch.itemCount < node.itemCount)
						{
							BranchItem item = node.itemList.back();
							newbranch.InsertInBranch(item);
							node.itemCount--;
							node.itemList.pop_back();
						}
						newbranch.ptrFather = Node::nodeCount;
						newbranch.attrLength = indexInfo.length;
						//newroot
						newroot.isRoot = true;
						BranchItem item;
						list<BranchItem>::iterator i;
						i = node.itemList.begin();
						item.key = (*i).key;
						item.ptr = nodeNo;
						newroot.InsertInBranch(item);
						i = newbranch.itemList.begin();
						item.key = (*i).key;
						item.ptr = Node::nodeCount-1;
						newroot.InsertInBranch(item);
						newroot.ptrFather = Node::nodeCount;
						newroot.attrLength = indexInfo.length;
							
						return breturn;
					}//if node is root
					//node is not root : cascade, add 1 branch and 1 branchItem
					else{		
						indexInfo.nodeCount += 1;
						fileName = itoa(Node::nodeCount+1,no,10);
						fileName += ".NODE";
						p = buff.read(fileName);
						Branch newbranch(p,true);
						while(newbranch.itemCount < node.itemCount)
						{
							BranchItem item = node.itemList.back();
							newbranch.InsertInBranch(item);
							node.itemCount--;
							node.itemList.pop_back();
						}
						newbranch.ptrFather = node.ptrFather;						
						newbranch.attrLength = indexInfo.length;

						list<BranchItem>::iterator i = newbranch.itemList.begin();
						breturn.key = (*i).key;
						breturn.ptr = Node::nodeCount;
						return breturn;
					}
				}//split occurs
				//branch after insertion is not full
				else{
					return breturn;
				}
			}//split occurs in lower level
	}//not leaf
	return breturn;
}
*/
/*
	string  IndexManager::DeleteValue(Index& indexInfo, string key, int nodeNo=1){
		char num[4];
		string fileName = indexInfo.name + itoa(nodeNo,num,10) + ".NODE";
		void* pb = buff.read(fileName);
		char* pv = static_cast<char*>(pb);
		string pass = "";

		//node is leaf
		if(*(pv+1) == '1')
		{
			Leaf node(pb,false);
			node.DeleteFromLeaf(key);
			if(node.itemCount < indexInfo.maxItems/2)
			{
				//if node is root
				if(node.isRoot){return pass;}
				//node is not root
				else
				{
					string parent = makeFileName(node.ptrFather,0);
					void* par = buff.read(parent);
					Branch father(par,false);
					list<BranchItem>::iterator i;
					for(i = father.itemList.begin(); i != father.itemList.end(); i++)
					{
						if((*i).ptr = nodeNo)
							break;
					}
					//find siblings
					int lsibling, rsibling;
					string sibFile;
					void* ptos;//pointer to sibling
					lsibling = rsibling = -1;
					if(i == father.itemList.begin())
					{
						if(father.itemList.size() >= 2)
						{
							i++;
							rsibling = (*i).ptr;
						}
						else{}
					}
					else
					{
						if(i == father.itemList.end())//TODO : may have to change
						{
							if(father.itemList.size() >= 2)
							{
								i--;
								lsibling = (*i).ptr;
							}
							else{}
						}
						else
						{
							i--;
							lsibling = (*i).ptr;
							i++;
							i++;
							rsibling = (*i).ptr;
						}
					}
					//check siblings
					//if has left sibling
					if(lsibling != -1)
					{
						sibFile = makeFileName(lsibling,0);
						ptos = buff.read(sibFile);
						Leaf lefts(ptos,false);
						LeafItem temp;
						//if can  coalesce
						if(lefts.itemCount + node.itemCount <= indexInfo.maxItems)
						{
							list<LeafItem>::iterator i = node.itemList.begin();
							pass = (*i).key;
							for( ; i != node.itemList.end(); i++)
							{
								temp = (*i);
								lefts.InsertInLeaf(temp);
								node.itemCount--;
								node.itemList.pop_front();
							}
							lefts.rightSibling = node.rightSibling;
						}
						//redistribute
						else
						{
							while(node.itemCount < lefts.itemCount)
							{
								temp = lefts.itemList.back();
								node.InsertInLeaf(temp);
								lefts.itemCount--;
								lefts.itemList.pop_back();
							}
							//TODO : update ptr to node in father
						}
						return pass;
					}//have left sibling
					//if no leftsibling, has right sibling
					if(rsibling != -1)
					{
						sibFile = makeFileName(rsibling,0);
						ptos = buff.read(sibFile);
						Leaf rights(ptos,false);
						LeafItem temp;
						//if can  coalesce
						if(rights.itemCount + node.itemCount <= indexInfo.maxItems)
						{
							i = rights.itemList.begin();
							pass = (*i).key;
							for( ; i != rights.itemList.end(); i++)
							{
								temp = (*i);
								node.InsertInBranch(temp);
								rights.itemCount--;
								rights.itemList.pop_front();
							}
							node.rightSibling = rights.rightSibling;
						}
						//redistribute
						else
						{
							while(node.itemCount < rights.itemCount)
							{
								temp = rights.itemList.front();
								node.InsertInBranch(temp);
								rights.itemCount--;
								rights.itemList.pop_front();
							}
							//TODO : update ptr to node in father
						}
						return pass;
					}//have rightsibling
					//if no left sibling or right sibling
					return pass;
				}//node branch is not root
			}//node.itemCount < indexInfo.maxItems/2
			//do not split
			else{return pass};
		}//node is leaf

		//node is branch:go down
		else
		{
			Branch node(pb,false);
			list<BranchItem>::iterator i= node.itemList.begin();
			//too small
			if(key < (*i).key)
			{
				printf("ERROR: DELETE A KEY SMALLER THAN THE SMALLEST IN NODE\n");
				exit(0);
			}
			for( ; i != node.itemList.end(); i++)
			{
				if(key < (*i).key)
					break;
			}
			i--;
			pass = DeleteValue(indexInfo,key,(*i).ptr);

			//if split doesn't occur in lower level
			if(pass =="")
			{
				return pass;
			}
			//split occurred in leaf 
			else
			{
				node.DeleteFromBranch(key);
				//too few items
				if(node.itemCount < indexInfo.maxItems/2)
				{
					//if node is root
					if(node.isRoot)
					{
						//1 child
						if(node.itemCount == 1)
						{
							list<BranchItem>::iterator i = node.itemList.begin();
							fileName = itoa((*i).ptr,num,10);
							fileName += ".NODE";
							p = buff.read(fileName);
							Branch newroot(p,false);//nonempty
							newroot.isRoot = true;
							buff.unlock(fileName);
							//TODO: delete old root
							return;
						}
						//>=2 child
						else{return;}
					}
					//node is not root
					else
					{
						string parent = makeFileName(node.ptrFather,0);
						void* par = buff.read(parent);
						Branch father(par,false);
						list<BranchItem>::iterator i;
						for(i = father.itemList.begin(); i != father.itemList.end(); i++)
						{
							if((*i).ptr = nodeNo)
								break;
						}
						//find siblings
						int lsibling, rsibling;
						string sibFile;
						void* ptos;//pointer to sibling
						lsibling = rsibling = -1;
						if(i == father.itemList.begin())
						{
							if(father.itemList.size() >= 2)
							{
								i++;
								rsibling = (*i).ptr;
							}
							else{}
						}
						else
						{
							if(i == father.itemList.end())//TODO : may have to change
							{
								if(father.itemList.size() >= 2)
								{
									i--;
									lsibling = (*i).ptr;
								}
								else{}
							}
							else
							{
								i--;
								lsibling = (*i).ptr;
								i++;
								i++;
								rsibling = (*i).ptr;
							}
						}
						//check siblings
						//if has left sibling
						if(lsibling != -1)
						{
							sibFile = makeFileName(lsibling,0);
							ptos = buff.read(sibFile);
							Branch lefts(ptos,false);
							BranchItem temp;
							//if can  coalesce
							if(lefts.itemCount + node.itemCount <= indexInfo.maxItems)
							{
								i = node.itemList.begin();
								pass = (*i).key;
								for( ; i != node.itemList.end(); i++)
								{
									temp = (*i);
									lefts.InsertInBranch(temp);
									node.itemCount--;
									node.itemList.pop_front();
								}
							}
							//redistribute
							else
							{
								while(node.itemCount < lefts.itemCount)
								{
									temp = lefts.itemList.back();
									node.InsertInBranch(temp);
									lefts.itemCount--;
									lefts.itemList.pop_back();
								}
								//TODO : update ptr to node in father
							}
							return pass;
						}//have left sibling
						//if no leftsibling, has right sibling
						if(rsibling != -1)
						{
							sibFile = makeFileName(rsibling,0);
							ptos = buff.read(sibFile);
							Branch rights(ptos,false);
							BranchItem temp;
							//if can  coalesce
							if(rights.itemCount + node.itemCount <= indexInfo.maxItems)
							{
								i = rights.itemList.begin();
								pass = (*i).key;
								for( ; i != rights.itemList.end(); i++)
								{
									temp = (*i);
									node.InsertInBranch(temp);
									rights.itemCount--;
									rights.itemList.pop_front();
								}
							}
							//redistribute
							else
							{
								while(node.itemCount < rights.itemCount)
								{
									temp = rights.itemList.front();
									node.InsertInBranch(temp);
									rights.itemCount--;
									rights.itemList.pop_front();
								}
								//TODO : update ptr to node in father
							}
							return pass;
						}//have rightsibling
						//if no left sibling or right sibling
						return pass;
					}//node branch is not root
				}//node.itemCount < indexInfo.maxItems/2

				//node adequate items
				else{
					return pass;
				}
			}//split occurred in leaf 
		}//node is branch: go down
	}

//if type == 0 : node, type == 1 table  
string makeFileName(int no, int type, string path = "", string name = "")
{
	char num[4];
	itoa(no,num,10);
	//NODE
	if(type == 0)
	{
		if(path != "")
			return path + '\\' + num + ".NODE";
		else
			return string(num) + ".NODE";
	}
	//TABLE
	else
	{
		if(path != "")
			return path + '\\' + name + ".TABLE";
		else
			return name + ".TABLE";
	}

}
*/
//==================global======================