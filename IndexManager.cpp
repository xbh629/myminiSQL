#define _CRT_SECURE_NO_WARNINGS
#include "BufferManager.hpp"
#include "CatalogManager.hpp"
#include "RecordManager.hpp"
#include "RecordRelationManager.hpp"
#include "IndexManager.h"
#include <iostream>
#include <string>

int byteInt(byte* start)
{
	byte bytes[4] = {*start, *(start+1), *(start+2), *(start+3)};
	int i = bytes[0]&0xFF;
	i |= ((bytes[1]<<8)&0xFF00);
	i |= ((bytes[2]<<16)&0xFF0000);
	i |= ((bytes[3]<<24)&0xFF000000);
	return i;
}

Branch createBranch(const record::DataPtr block, const catalog::Field *field, bool blank){
	Node node;
	std::list<BranchItem> itemList;
	
	if(!blank)
	{
		byte* ptr = block;
		memcpy(&node, ptr, sizeof(node)- sizeof(catalog::Field*)-sizeof(record::DataPtr));
		node.field = field;
		node.ptrBlock = block;
		ptr += sizeof(Node);
		for(size_t i = 0; i < node.itemCount; i++){
			BranchItem item;
			memcpy(&item, ptr, sizeof(BranchItem));
			itemList.push_back(item);
			ptr += sizeof(BranchItem);
		}
	}
	else
	{
		node.isRoot = false;
		node.isLeaf = false;
		node.itemCount = 0;
		node.ptrFather = -1;
		node.field = field;
		node.ptrBlock = block;
	}
	return Branch(node, itemList);
}

Leaf createLeaf(const record::DataPtr block, const catalog::Field *field, bool blank){
	Node node;
	std::list<record::DataAndLoc> itemList;
	int leftSib, rightSib;
	if(!blank)
	{
		byte* ptr = block;
		memcpy(&node, ptr, sizeof(node)- sizeof(catalog::Field*)-sizeof(record::DataPtr));
		node.field = field;
		node.ptrBlock = block;
		ptr += sizeof(Node);
		memcpy(&leftSib, ptr, sizeof(int));
		ptr += sizeof(int);
		memcpy(&rightSib, ptr, sizeof(int));
		ptr += sizeof(int);
		for(size_t i = 0; i < node.itemCount; i++){
			record::DataAndLoc item;
			memcpy(&item, ptr, sizeof(record::DataAndLoc));
			itemList.push_back(item);
			ptr += sizeof(record::DataAndLoc);
		}
	}
	else
	{
		node.isLeaf = true;
		node.isRoot = false;
		node.itemCount = 0;
		node.ptrFather = -1;
		node.field = field;
		node.ptrBlock = block;
		leftSib = -1;
		rightSib = -1;
	}
	return Leaf(node,leftSib,rightSib,itemList);
}

void makeFile(std::string name)
{
	std::fstream fout(name.c_str(), ios::out);
	fout.close();
}

int CompareDataUnit(DataUnit& data1, DataUnit& data2, catalog::Field& field)
{
	switch (field.type)
	{
	case catalog::Field::INT:{return data1.integer- data2.integer;}
	case catalog::Field::FLOAT:{return data1.fl - data2.fl;}
	case catalog::Field::CHARS:{return strncmp(data1.str, data2.str,field.char_n);}
	}
}
//====================================================================
/*
int Node::getInt(record::DataPtr block,int start){
	byte *p = block;
	p += start;
	return byteInt(p);
}
*/
//TODO
void Branch::InsertInBranch(BranchItem item)
{
	itemCount++;
	std::list<BranchItem>::iterator i;
	if(itemList.size() == 0)//TODO
		itemList.push_front(item);
	else{
		for(i = itemList.begin();i != itemList.end(); i++)
		{
			if(CompareDataUnit((*i).key, item.key, *(this->field)) > 0)break;
		}
		if(i != itemList.end())
			itemList.insert(i,item);
		else
			itemList.push_back(item);
	}
}

void Branch:: DeleteFromBranch(DataUnit key){
	std::list<BranchItem>::iterator i;
	if(itemList.size() == 0)
		printf("key is not in branch.\n");
	else{
		for(i = itemList.begin(); i != itemList.end(); i++)
		{
			if(CompareDataUnit(i->key, key, *(this->field)) == 0)
			{
				itemList.erase(i);
				itemCount--;
				return;
			}
		}
	}
}
//TODO
Branch::~Branch(){
	byte* ptr = this->ptrBlock;
	memcpy(ptr, &(this->isRoot), sizeof(bool));ptr += sizeof(bool);
	memcpy(ptr, &(this->isLeaf), sizeof(bool));ptr += sizeof(bool);
	memcpy(ptr, &(this->itemCount), sizeof(int));ptr += sizeof(int);
	memcpy(ptr, &(this->ptrFather), sizeof(int)); ptr += sizeof(int);
	//memcpy(ptr, &(this->field), sizeof(catalog::Field*)); ptr += sizeof(catalog::Field*);
	//memcpy(ptr, &(this->ptrBlock), sizeof(record::DataPtr)); ptr += sizeof(record::DataPtr);
	std::list<BranchItem>::iterator i;
	for(i = itemList.begin(); i != itemList.end(); i++)
	{
		memcpy(ptr, &(i->key), sizeof(DataUnit));ptr += sizeof(DataUnit);
		memcpy(ptr, &(i->ptr), sizeof(int)); ptr += sizeof(int);
	}
	//unlock or write?
}

void Leaf::InsertInLeaf(record::DataAndLoc item){
	itemCount++;
	std::list<record::DataAndLoc>::iterator i;
	if(itemList.size() == 0)
		itemList.push_front(item);
	else{
		for(i = itemList.begin();i != itemList.end(); i++)
		{
			if(CompareDataUnit(i->data, item.data, *(this->field)) > 0)break;
		}
		if(i != itemList.end())
			itemList.insert(i,item);
		else
			itemList.push_back(item);
	}
}

void Leaf::DeleteFromLeaf(DataUnit key){
	std::list<record::DataAndLoc>::iterator i;
	if(itemList.size() == 0)
		printf("key is not in leaf.\n");
	else
	{
		for(i = itemList.begin(); i != itemList.end(); i++)
		{
			if(CompareDataUnit(i->data, key, *(this->field)) == 0)
			{
				itemList.erase(i);
				itemCount--;
				return;
			}
		}
	}
}
//TODO
Leaf::~Leaf(){
	byte* ptr = this->ptrBlock;
	memcpy(ptr, &(this->isRoot), sizeof(bool));ptr += sizeof(bool);
	memcpy(ptr, &(this->isLeaf), sizeof(bool));ptr += sizeof(bool);
	memcpy(ptr, &(this->itemCount), sizeof(int));ptr += sizeof(int);
	memcpy(ptr, &(this->ptrFather), sizeof(int)); ptr += sizeof(int);
	//memcpy(ptr, &(this->field), sizeof(catalog::Field*)); ptr += sizeof(catalog::Field*);
	//memcpy(ptr, &(this->ptrBlock), sizeof(record::DataPtr)); ptr += sizeof(record::DataPtr);
	memcpy(ptr, &leftSibling, sizeof(int)); ptr += sizeof(int);
	memcpy(ptr, &rightSibling,sizeof(int)); ptr += sizeof(int);
	std::list<record::DataAndLoc>::iterator i;
	for(i = itemList.begin(); i != itemList.end(); i++)
	{
		memcpy(ptr, &(i->data), sizeof(DataUnit));ptr += sizeof(DataUnit);
		memcpy(ptr, &(i->filePtr), sizeof(int)); ptr += sizeof(record::FilePtr);
	}
//TODO unlock or write??
}
//-----------------------------index manager
//TODO extract and fill in
void IndexManager::CreateIndex(const std::string& indexName, const std::string& relationName, const std::string& attributeName){
	auto& IndexInfo = catam->getIndexInfo(indexName);
	std::string path = "Index\\" + indexName + '_';
	mkdir("Index\\");
	//root node for reference
	std::string fileName = "0.NODE";
	makeFile(path + fileName);
	FILE* rf = fopen((path + fileName).c_str(),"rw");
	fprintf(rf,"%d_%d",1,1);//root_max
	fclose(rf);
	//makae 1st leaf (actual root)
	
	fileName = "1.NODE";
	makeFile(path + fileName);
	record::DataPtr p = buffm->read(path + fileName);
	Leaf root = createLeaf(p,IndexInfo.field,true);
	root.isRoot = true;
	//root.inmanag = this;

	//---get data from table and insert in B+ tree
	RecordManager::RecordSet tuples = recm->select(relationName);
	std::list<record::Tuple>::iterator t;
	for(t = tuples.begin(); t != tuples.end(); t++)
	{
		//TODO: parse and insert
	}
}
//TODO binary read
void IndexManager::DropIndex(const std::string& indexName, const std::string& relationName, const std::string& attributeName){
	std::string path = "Index\\" + indexName + "_0.NODE";
	FILE* fp = fopen(path.c_str(),"rw");
	int total;
	fscanf(fp,"%d",&total);
	fgetchar();
	fscanf(fp,"%d",&total);//max

	path = "Index\\" + indexName + '_';
	std::string fn;
	char no[4];
	for(int i = 0; i <= total; i++)
	{
		itoa(i,no,10);
		fn = no;
		fn += ".NODE";
		remove((path+fn).c_str());
	}
}
//TODO
IndexManager::PosSet IndexManager::EqualQuery(const std::string& indexName,const std::string& relationName, DataUnit key, int nodeNo){
	char no[4];
	IndexManager::PosSet freturn;
	catalog::IndexInfo indInfo = catam->getIndexInfo(indexName);

	std::string path = "Index\\" + indexName + '_';
	std::string fileName = itoa(nodeNo,no,10);
	fileName += ".NODE";
	if(nodeNo == 0)//go to root
	{
		FILE* fp = fopen((path+fileName).c_str(),"rb");
		int go;
		fscanf(fp,"%d",&go);
		fclose(fp);
		freturn = EqualQuery(indexName, relationName, key, go);
		return freturn;
	}
	else
	{
		record::DataPtr p = buffm->read(path + fileName);
		bool isl = *(p+1);
		//reach leaf
		if(isl)
		{
			Leaf node = createLeaf(p,indInfo.field,false);
			std::list<record::DataAndLoc>::iterator i;
			for(i = node.itemList.begin(); i != node.itemList.end(); i++)
			{
				//get data from table
				if(CompareDataUnit(i->data, key, *(node.field)) == 0)
				{
					freturn.push_back(i->filePtr);
				}
			}
			return freturn;
		}
		//go down
		else
		{
			Branch node = createBranch(p,indInfo.field,false);
			std::list<BranchItem>::iterator i;
			i++;//skip 1st pointer
			for(; i != node.itemList.end(); i++)		
			{
				if(CompareDataUnit(i->key, key, *(node.field)) > 0){break;}
			}
			i--;
			freturn = EqualQuery(indexName,relationName,key,(*i).ptr);
			return freturn;
		}
	}
}
//TODO
IndexManager::PosSet RangeQuery(const std::string& indexName, const std::string& relationName, DataUnit start, DataUnit end, int nodeNo)
{
}
//TODO
BranchItem  IndexManager::InsertValue(const catalog::IndexInfo& indInfo,record::DataAndLoc item, int nodeNo){	
	BranchItem breturn;
	char no[4];
	std::string path = "Index\\"+ indInfo.index_name + '_';
	std::string fileName = itoa(nodeNo,no,10);
	fileName += ".NODE";
	static int curRoot, curMax;
	static std::string tableName;

	if(nodeNo == 0)//go to root
	{
		tableName = (*indInfo.relation).name;
		FILE* fp = fopen((path+fileName).c_str(),"rb");
		//int go;
		fscanf(fp,"%d",&curRoot);
		//curRoot = go;
		fgetchar();
		fscanf(fp,"%d",&curMax);
		breturn = InsertValue(indInfo, item, curRoot);
		fp = fopen((path+fileName).c_str(),"wb");
		fprintf(fp,"%d_%d",curRoot,curMax);
		fclose(fp);
		return breturn;
	}
	else
	{
		record::DataPtr p = buffm->read(path + fileName);
		bool isl = *(p+1);
		int maxItems = 4096/(sizeof(record::DataAndLoc));
		int newr,newmax;//to update 0.NODE
		//node is leaf
		if(isl){
			Leaf node= createLeaf(p,indInfo.field,false);
			node.InsertInLeaf(item);
			//if node is full after insertion, split
			if(node.itemList.size() > maxItems)
			{
				//node is root : add 1 leaf and 1 branch
				if(node.isRoot)
				{
					//make new leaf(blank)
					curMax ++;
					fileName = itoa(curMax,no,10);
					fileName += ".NODE";
					makeFile(fileName);
					p = buffm->read(path+fileName);
					Leaf newleaf= createLeaf(p,indInfo.field,true);
					//make new branch(blank)
					curMax ++;
					fileName = itoa(curMax,no,10);
					fileName += ".NODE";
					makeFile(fileName);
					p = buffm->read(path+fileName);
					Branch newroot=createBranch(p,indInfo.field,true);
					curRoot = curMax;

					//adjust leaves
					while(node.itemCount > ceil((float)maxItems/2))//stop when ceil(m/2) entries remain in node
					{
						record::DataAndLoc item = node.itemList.back();
						newleaf.InsertInLeaf(item);
						node.itemCount--;
						node.itemList.pop_back();
					}
					newleaf.leftSibling = nodeNo;//current node
					newleaf.ptrFather = curRoot;//after making new root
					newleaf.rightSibling = node.rightSibling;

					node.isRoot = false;
					node.ptrFather = newleaf.ptrFather;
					node.rightSibling = curMax - 1;

					//make new root
					newroot.isRoot = true;
					newroot.ptrFather = curRoot;//point to self
					BranchItem bitem;
					bitem.key = "";//for first item in branch
					bitem.ptr = nodeNo;//use only the pointer
					newroot.InsertInBranch(bitem);
					std::list<record::DataAndLoc>::iterator i = newleaf.itemList.begin();
					bitem.key = i->data;
					bitem.ptr = curMax - 1;
					newroot.InsertInBranch(bitem);
					breturn.pt=-1;
					return breturn;
				}
				//full leaf not root : cascade, add 1 leaf and 1 branchitem
				else{
					curMax ++;
					//make new leaf(blank)
					fileName = itoa(curMax,no,10);
					fileName += ".NODE";
					makeFile(path + fileName);
					p = buffm->read(fileName);
					Leaf newleaf= createLeaf(p,indInfo.field,true);
					while(node.itemCount > ceil((float)maxItems/2))
					{
						record::DataAndLoc item = node.itemList.back();
						newleaf.InsertInLeaf(item);
						node.itemCount--;
						node.itemList.pop_back();
					}
					newleaf.leftSibling = nodeNo;
					newleaf.ptrFather = node.ptrFather;
					newleaf.rightSibling = node.rightSibling;
					node.rightSibling = curMax;
					std::list<record::DataAndLoc>::iterator i = newleaf.itemList.begin();
					breturn.key = i->data;
					breturn.ptr = curMax;
					return breturn;
				}
			}//leaf is full
			//leaf is not full after insertion
			else{
				breturn.ptr=-1;
				return breturn;
			}
		}
		//node is branch
		else{
			Branch node=createBranch(p,indInfo.field,false);//make a nonempty branch
			std::list<BranchItem>::iterator i = node.itemList.begin();
			i++;//skip first entry without key
			for(i; i != node.itemList.end(); i++)
			{
				if(CompareDataUnit(item.data, i->key, *node.field) < 0)
					break;
			}
			i--;
			BranchItem ret = InsertValue(indInfo,item,(*i).ptr);
				
			//split doesn't occur
			if(ret.ptr == -1){
				breturn.ptr=-1;
				return breturn;
			}
			//split occurs
			else{
				node.InsertInBranch(ret);
				//node is full after insertion
				if(node.itemCount > maxItems)
				{
					//node is root : add 2 branch
					if(node.isRoot)
					{
						curMax++;
						fileName = itoa(curMax,no,10);
						fileName += ".NODE";
						makeFile(fileName);
						p = buffm->read(fileName);
						Branch newbranch=createBranch(p,indInfo.field,true);
						curMax++;
						fileName = itoa(curMax,no,10);
						fileName += ".NODE";
						makeFile(fileName);
						p = buffm->read(fileName);
						Branch newroot=createBranch(p,indInfo.field,true);
						curRoot = curMax;
						
						//newbranch
						while(node.itemCount > ceil((float)maxItems/2))
						{
							BranchItem item = node.itemList.back();
							//change ptrFather of leaves that go to new branch!
							fileName = itoa(item.ptr,no,10);
							fileName += ".NODE";
							p = buffm->read(fileName);
							Leaf change=createLeaf(p,indInfo.field,false);
							change.ptrFather = curMax-1;//w&&&&&&&&&&&&&&&&rite to buffer
							newbranch.InsertInBranch(item);
							node.itemCount--;
							node.itemList.pop_back();
						}
						newbranch.ptrFather = curMax;
						//node
						node.isRoot = false;
						node.ptrFather = newbranch.ptrFather;
						//newroot
						newroot.isRoot = true;
						newroot.ptrFather = curMax;
						BranchItem item;
						item.key = "";
						item.ptr = nodeNo;
						newroot.InsertInBranch(item);
						i = newbranch.itemList.begin();
						item.key = (*i).key;
						item.ptr = curMax-1;
						newroot.InsertInBranch(item);
						(*i).key="";//
						breturn.ptr=-1;
						return breturn;
					}//if branch is root
					//full branch is not root : cascade, add 1 branch and 1 branchItem
					else{		
						curMax++;
						fileName = itoa(curMax,no,10);
						fileName += ".NODE";
						makeFile(fileName);
						p = buffm->read(fileName);
						Branch newbranch(p,this,true);
						while(node.itemCount > ceil((float)maxItems/2))
						{
							BranchItem item = node.itemList.back();
							//change ptrFather of leaves that go to new branch!
							fileName = itoa(item.ptr,no,10);
							fileName += ".NODE";
							p = buffm->read(fileName);
							Leaf change(p,this,false);
							change.ptrFather = curMax;//w&&&&&&&&&&&&&&&&rite to buf
							newbranch.InsertInBranch(item);
							node.itemCount--;
							node.itemList.pop_back();
						}
						newbranch.ptrFather = node.ptrFather;			

						std::list<BranchItem>::iterator i = newbranch.itemList.begin();
						breturn.key = (*i).key;
						breturn.ptr = curMax;
						(*i).key="";//left most pointer with no key before
						return breturn;
					}
				}//split occurs
				//branch after insertion is not full
				else{
					breturn.ptr=-1;
					return breturn;
				}
			}//split occurs in lower level
		}//node is branch
	}
}
//TODO
/*
BranchItem  IndexManager::DeleteValue(Index& indexInfo, string key, int nodeNo){
	char no[4];
	string fileName = itoa(nodeNo,no,10);
	fileName += ".NODE";
	void* p = buff.read(fileName);
	char* pv = static_cast<char*>(p);
	//string pass = "";//-------------------------------------------------------------------------?
	BranchItem breturn;
	list<LeafItem>::iterator il;
	list<BranchItem>::iterator ib;

	//node is leaf
	if(*(pv+1) == '1')
	{
		Leaf node(p,false);
		node.attrLength = indexInfo.length;
		node.DeleteFromLeaf(key);
		if(node.itemCount < ceil((float)indexInfo.maxItems/2))
		{
			//leaf is root
			if(node.isRoot){ return breturn;}
			//node is not root
			else
			{
				//find left&right sibling
				int lsibling, rsibling;
				if(node.leftSibling != nodeNo && node.leftSibling != -1)
				{
					fileName = itoa(node.leftSibling,no,10);
					fileName += ".NODE";
					p = buff.read(fileName);
					Leaf templ(p,false);
					if(templ.ptrFather == node.ptrFather)
						lsibling =1;
					else lsibling = 0;
				}
				if(node.rightSibling != nodeNo && node.rightSibling != -1)//assume -1,-1 
				{
					fileName = itoa(node.rightSibling,no,10);
					fileName += ".NODE";
					p = buff.read(fileName);
					Leaf tempr(p,false);
					if(tempr.ptrFather == node.ptrFather)
						rsibling =1;
					else rsibling = 0;
				}
				//if has left sibling
				list<LeafItem>::iterator i;
				LeafItem temp;
				if(lsibling)
				{
					fileName = itoa(node.leftSibling,no,10);
					fileName += ".NODE";
					p = buff.read(fileName);
					Leaf lefts(p,false);
					//if can coalesce
					if(lefts.itemCount + node.itemCount <= indexInfo.maxItems)
					{
						for( i = node.itemList.begin(); i != node.itemList.end(); i++)
						{
							temp = (*i);
							lefts.InsertInLeaf(temp);
							node.itemCount--;
							node.itemList.pop_front();
						}
						lefts.rightSibling = node.rightSibling;
						breturn.ptr = nodeNo;//here for delete
						return breturn;
					}
					//redistribute
					else
					{
						i = lefts.itemList.end();
						i--;
						for(; node.itemCount < ((float)indexInfo.maxItems/2);i--)//----------------------------------------
						{
								temp = (*i);
								node.InsertInLeaf(temp);
								lefts.itemCount--;
								lefts.itemList.pop_back();
						}
						breturn.ptr = nodeNo;
						i = node.itemList.begin();
						breturn.key = (*i).key;//here for update
						return breturn;
					}
				}//have left sibling
				//leftsibling, has right sibling
				if(rsibling)
				{
					fileName = itoa(node.rightSibling,no,10);
					fileName += ".NODE";
					p = buff.read(fileName);
					Leaf rights(p,false);
					//if can  coalesce
					if(rights.itemCount + node.itemCount <= indexInfo.maxItems)
					{
						for( i = rights.itemList.begin(); i != rights.itemList.end(); i++)
						{
							temp = (*i);
							node.InsertInLeaf(temp);
							rights.itemCount--;
							rights.itemList.pop_front();
						}
						node.rightSibling = rights.rightSibling;
						breturn.ptr = node.rightSibling;//here for delete
						return breturn;
					}
					//redistribute
					else
					{
						for(i = rights.itemList.begin(); node.itemCount < ((float)indexInfo.maxItems/2);i++)//----------------------------------------
						{
							temp = (*i);
							node.InsertInLeaf(temp);
							rights.itemCount--;
							rights.itemList.pop_front();
						}
						breturn.ptr = node.rightSibling;
						i = rights.itemList.begin();
						breturn.key = (*i).key;//here for update
						return breturn;
					}
				}//have rightsibling
				//if no left sibling or right sibling
				return breturn;
			}//leaf is not root
		}//leaf.itemCount < indexInfo.maxItems/2
		//leaf not too small
		else{return breturn;}
	}//node is leaf

	//node is branch:go down
	else
	{
		Branch node(p,false);
		list<BranchItem>::iterator i= node.itemList.begin();
		i++;//skip 1st entry
		//too small
		for(; i != node.itemList.end(); i++)
		{
			if(key < (*i).key)
				break;
		}
		i--;
		BranchItem ret = DeleteValue(indexInfo,key,(*i).ptr);

		//if no deletion or adjust required
		if(ret.key =="" && ret.ptr == 0){return breturn;}
		//deletion or adjust  
		else
		{
			if(ret.key =="")//delete
			{
				for(ib = node.itemList.begin(); ib!= node.itemList.end(); ib++)
				{
					if((*ib).ptr == ret.ptr)
					{
						node.DeleteFromBranch((*ib).key);
						break;
					}
				}
			}
			else//update
			{
				for(ib = node.itemList.begin(); ib!= node.itemList.end(); ib++)
				{
					if((*ib).ptr == ret.ptr)
					{
						(*ib).key = ret.key;
						break;
					}
				}
			}
			//too few items
			if(node.itemCount < ceil((float)indexInfo.maxItems/2))
			{
				//if node is root
				if(node.isRoot)
				{
					//1 child
					if(node.itemCount == 1)
					{
						ib = node.itemList.begin();
						fileName = itoa((*ib).ptr,no,10);
						fileName += ".NODE";
						p = buff.read(fileName);


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
	*/
