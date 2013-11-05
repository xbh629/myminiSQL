#ifndef _INDEXMANAGER_H_
#define _INDEXMANAGER_H_
#include "ClassDefinition.h"
#include "BufferManager.h"
#include <direct.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
//#include <Shlwapi.h>//used when droppin index(delete folder and its files)
//#pragma comment(lib,"Shlwapi.lib") 

//extern CatalogManager catalog;
using namespace std;
//using namespace buffer;

//extern BufferManager buff;
//==============================for IndexManager====================
class LeafItem{
public:
	string key;
	int blockNo;
	int offset;
	LeafItem():key(""),blockNo(0),offset(0){}
	LeafItem(string k, int blockNo, int offset):key(k),blockNo(blockNo),offset(offset){}
};

class BranchItem{
public:
	string key;
	int ptr;
	BranchItem():key(""),ptr(0){}
	BranchItem(string k, int ptr):key(k),ptr(ptr){}
};

class Node{
	friend class IndexManager;
public:
	bool isRoot;
	bool isLeaf;
	int itemCount;
	int ptrFather;
	int attrLength;
	void* ptrBlock;
	Node(){nodeCount++;}
	~Node(){}
	int getPointer(void* block,int start);
	int getNumber(void* block, int start);
	static int nodeCount;
};

class Branch : public Node{
public:
	list<BranchItem> itemList;
	Branch(){}
	Branch(void* block, bool blank);
	void InsertInBranch(BranchItem item);
	void DeleteFromBranch(string key);
	//TODO: write to buffer block (filePath to be determined)
	~Branch();
};

class Leaf : public Node{
public:
	int leftSibling;
	int rightSibling;
	list<LeafItem> itemList;
	Leaf(){}
	Leaf(void* block, bool blank);
	void InsertInLeaf(LeafItem item);
	void DeleteFromLeaf(string key);
	//TODO: (filePath to be determined)
	~Leaf();
};  

/*
class Index
{
public:
	string name;
	string table;
	string attribute;
	int length;
	int nodeCount;
	int maxItems;//how many items may be in a node
	Index():name(""),table(""),attribute(""),length(0),nodeCount(0),maxItems(0){}
	Index(string name, string table, string attribute){
		name = name;
		table = table;
		attribute = attribute;
		//length = catalog.getAttrLength(table,attribute);
		nodeCount = 0;
		maxItems = (4096-15)/(length + 2* POINTERSIZE);//estimated by assuming leaf node
	}
};
*/
//==============================for IndexManager====================

string makeFileName(string path, int no, string type);
//NOTICE: format of folder \Index\tableName\indexName
//NOTICE: format of node   \Index\tableName\indexName\  nodeNo.NODE
//NOTICE: format of table  \tableName.TABLE

//====== TODO: may change all fileName in each read() to path+fileName
/*
class IndexManager{
public:
	//B+树的创建 TODO: get tuple from table(how is table stored?)
	void CreateIndex(const Table& tableInfo, Index& indexInfo);
	//删除（由索引的定义与删除引起）TODO: delete a folder and files in it
	void DropIndex(Index& indexInfo);
	//等值查找 TODO: assemble result
	Result EqualQuery(const Table& tableInfo,const Index& indexInfo, string key, int nodeNo=1);
	//插入键值 TODO: update after insertion
	BranchItem InsertValue(Index& indexInfo,LeafItem item, int nodeNo=1);
	//删除键值 TODO: 
	string DeleteValue(Index& indexInfo, string key, int nodeNo=1);
};
*/
#endif