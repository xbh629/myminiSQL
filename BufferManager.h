#pragma once
#include <string>
#include <list>
#include <cstdint>
#include "BufferManagerExn.h"

typedef uint8_t byte;

namespace buffer{
	struct Block{
		Block(const std::string& filePath, size_t index, bool pin, bool dirt)
			:filePath(filePath), index(index), pin(pin), dirt(dirt){}

		std::string filePath;
		size_t index;	//index inside memory => 0 until BLOCK_NUM
		bool pin;	//whether can be substitude
		bool dirt;	//whether has been written => set by write
	};
}

class BufferManager
{
	friend class IndexManager;
public:
	enum {BLOCK_SIZE = 4 * 1024}; //block_size = 4K
public: 
	BufferManager(void);
	virtual ~BufferManager(void);
	
	/**	change pin to prevent or release lock of that block
	*	available only when block exist in buffer
	*/
	bool lock(const std::string& filePath);
	bool unlock(const std::string& filePath);
	/** ADDED BY BJM (FOR USE IN INDEX)
	*	same as bool unlock(const std::string& filePath)
	*	except that void* is used as parameter
	*/
	bool unlock(void* ptrBlock)
	{
		auto block = findBlockByAddr(ptrBlock);
		std::string path = block->filePath;
		return unlock(path);
	}

	/**	read from file to memory, 
	*@return: the pointer to the address of block in the memory
	* this function will automatically lock the block and
	* user must release the block after read operation done
	* will not save any dirty data replaced in the buffer
	*/
	void* read(const std::string& filePath);

	/**
	* write data in memory
	* will not save to the file unless user call writeBack function
	* if block not in the list will load the data in and write it
	*/
	void write(const std::string& filePath, void *source);

	/**
	*	update data in a locked (ensured by user) place with its size 
	*	within a block
	*/
	void write(void *dest, void *source, size_t size);

	/**
	*	force some block inside memory save into files
	*	if not exists in buffer then ignore
	*	will automatically set dirt bit to false
	*/
	void writeBack(const std::string& filePath);

	/**ADDED BY BJM (MODIFIED FOR USE IN INDEX)
	*	force 1 block to be written to file
	*	automatically set dirt bit to false
	*	locate block by pointer
	*/
	void writeBack(void* source)
	{
		auto block = findBlockByAddr(source);
		std::string path = block->filePath;
		writeBack(path);
	}

private:
	enum {BLOCK_NUM = 32}; //number of blocks inside buffer
	typedef std::list<buffer::Block>::iterator block_iter;
	typedef std::list<buffer::Block>::const_iterator block_const_iter;
private:
	/** add a block inside buffer 
	*	didn't copy contents of file into memory
	*	should be ensured that the block is not in the buffer
	*/
	block_iter addBlock(const std::string& filePath);

	block_const_iter findBlockByAddr(void* addr) const;
	block_iter findBlockByAddr(void* addr);

	/** choose a block to substitude
	*	@return: if cannot find return rend
	*/
	block_iter LRU_substitution(void);
	block_const_iter LRU_substitution(void) const;

	/** pull the block data forward in the list
	*/
	void pullForward(block_iter ptrBlock){
		list.splice(list.begin(), list, ptrBlock);
	}

	/** get block inside buffer by its name
	*	@return: if not find then return list.end
	*/
	block_const_iter getBlock(const std::string& filePath) const;
	block_iter getBlock(const std::string& filePath);

	/** convert block inside buffer to its address
	*/
	void *getAddr(block_iter ptrBlock) const{
		return (byte *)contents + BLOCK_SIZE * ptrBlock->index;
	}
private:
	void *contents;	//the whole block data in memory
	std::list<buffer::Block> list; 
	//a list contain the info about block inside buffer
};
