#include "ClassDefinition.h"
#include "BufferManager.h"
#include "IndexManager.h"
//test 
int Node::nodeCount = 0;
BufferManager buff;

int main(void)
{

return 0;
}
/*
Node::getPointer,	Node::getNumber, 
Branch::Branch(true), Branch::Branch(false)
Branch::InsertInBranch,	Branch:: DeleteFromBranch
test code:
	BufferManager buff;
	void* con = buff.read("branch.NODE");
	Branch a(con,true);
	Branch b(con,false);
	Branch c(con,false);
	printf("delete from b:\n");
	b.DeleteFromBranch("aa");
	b.DeleteFromBranch("cc");
	b.DeleteFromBranch("bb");
	b.DeleteFromBranch("aa");
	printf("delete from c:\n");
	c.DeleteFromBranch("bb");
	c.DeleteFromBranch("dd");
*/
/*
Leaf::Leaf(true), Leaf::Leaf(false)
Leaf::InsertInLeaf,	Leaf:: DeleteFromLeaf
	BufferManager buff;
	void* con = buff.read("leaf.NODE");
	Leaf a(con,true);
	Leaf b(con,false);
	Leaf c(con,false);
	printf("delete from b:\n");
	b.DeleteFromLeaf("aa");
	b.DeleteFromLeaf("cc");
	b.DeleteFromLeaf("bb");
	b.DeleteFromLeaf("ee");
	printf("delete from c:\n");
	c.DeleteFromLeaf("bb");
	c.DeleteFromLeaf("dd");
*/
/*
test Node:: static int nodeNo
	printf("initialize : Node::nodeCount = %d\n",Node::nodeCount);
	Node n;
	printf("after making a node : Node::nodeCount = %d\n",Node::nodeCount);
	
	void* p = buf.read("leaf.NODE");
	Leaf l(p,true);
	printf("after making a leaf : Node::nodeCount = %d\n",Node::nodeCount);
	
	p = buf.read("branch.NODE");
	Branch b(p,true);
	printf("after making a branch : Node::nodeCount = %d\n",Node::nodeCount);
	
	char ch;
	ch = getchar();
*/
/*
test destructors : ~Branch() ~Leaf()
	void* p = buff.read("leaf.NODE");
	printf("modify leaf:\n");
	Leaf l(p,false);
	//MODIFY LEAF
	LeafItem li("xx",8,8);
	printf("before insert : itemCount = %d\n",l.itemCount);
	l.InsertInLeaf(li);
	l.leftSibling = 10;
	l.rightSibling = 20;
	printf("after insert : itemCount = %d\n",l.itemCount);

	p = buff.read("branch.NODE");
	printf("modify branch:\n");
	Branch b(p,false);
	//MODIFY BRANCH
	BranchItem bi("yy",9);
	printf("before insert : itemCount = %d\n",b.itemCount);
	b.InsertInBranch(bi);
	printf("after insert : itemCount = %d\n",b.itemCount);
*/