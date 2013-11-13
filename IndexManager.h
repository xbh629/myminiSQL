#ifndef _INDEXMANAGER_H_
#define _INDEXMANAGER_H_
#include "BufferManager.hpp"
#include "CatalogManager.hpp"
#include "RecordManager.hpp"
#include "RecordRelationManager.hpp"
#include <fstream>
#include <direct.h>
#include <iostream>
#include <string>
#include <list>

Branch createBranch(const record::DataPtr block,  const catalog::Field *field, bool blank);
Leaf createLeaf(const record::DataPtr block, const catalog::Field *field, bool blank);
void makeFile(std::string name);
int byteInt(byte* start);
//return difference
int CompareDataUnit(DataUnit& data1, DataUnit& data2, catalog::Field& type);
//==================================================================================
class BranchItem{
public:
	DataUnit key;
	int ptr;
	BranchItem(){}
	BranchItem(int pChild):key(),ptr(pChild){}//leftmost entry in a branch node
	BranchItem(DataUnit k, int ptr):key(k),ptr(ptr){}
};

class Node{
public:
	bool isRoot;
	bool isLeaf;
	int itemCount;
	int ptrFather;
	catalog::Field *field;
	record::DataPtr ptrBlock;
public:
	Node(){}
	~Node(){}
	int getInt(record::DataPtr block,int start);
};

class Branch : public Node{
public:
	std::list<BranchItem> itemList;
public:
	Branch(const Node& node, std::list<BranchItem> itemList):Node(node),itemList(itemList){}
	void InsertInBranch(BranchItem item);
	void DeleteFromBranch(DataUnit key);
	~Branch();
};

class Leaf : public Node{
public:
	int leftSibling;
	int rightSibling;
	std::list<record::DataAndLoc> itemList;
public:
	Leaf(const Node& node, int leftSibling, int rightSibling, std::list<record::DataAndLoc> itemList):Node(node),leftSibling(leftSibling), rightSibling(rightSibling), itemList(itemList){}
	void InsertInLeaf(record::DataAndLoc item);
	void DeleteFromLeaf(DataUnit key);
	~Leaf();
};  

class IndexManager
{
public:
	friend class Branch;
	friend class Leaf;
	typedef std::list<record::FilePtr> PosSet;
private:
	BufferManager *buffm;
	CatalogManager *catam;
	RecordManager * recm;

public:
	IndexManager(BufferManager& buff)
	{
		buffm = &buff;
		catam = new CatalogManager(buffm);
		recm = new RecordManager();
	}
	void CreateIndex(const std::string& indexName, const std::string& relationName, const std::string& attributeName);
	void DropIndex(const std::string& indexName, const std::string& relationName, const std::string& attributeName);
	PosSet EqualQuery(const std::string& indexName, const std::string& relationName, DataUnit key, int nodeNo=0);
	PosSet RangeQuery(const std::string& indexName, const std::string& relationName, DataUnit start, DataUnit end, int nodeNo=0);
	BranchItem InsertValue(const catalog::IndexInfo& indInfo, record::DataAndLoc item, int nodeNo=0);
	BranchItem DeleteValue(const catalog::IndexInfo& indInfo, record::DataAndLoc item, int nodeNo=0);
};

#endif