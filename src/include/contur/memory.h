#pragma once

#include "contur/architecture_cpu.h"
#include <vector>

class Memory
{
  private:
    int SIZE_OF_MEMORY_IN_BLOCKS;
    int currBlock;
    std::vector<Block> heap;

    void clearBlock(int addr);

  public:
    Memory(int SIZE_OF_MEMORY_IN_BLOCKS);

    void setAddrBlock(int addr)
    {
        currBlock = addr;
    }

    void setCmd(int cmd, int op1, int op2);

    Block &read(int addr)
    {
        return heap[addr];
    }

    int getAddrFreeBlock();
    int getNextAddr(int currAddr) const
    {
        return currAddr + 1;
    }

    void clearMemory();
    void clearMemory(int AddrFrom, int AddrTo);

    int getSizeMemory() const
    {
        return SIZE_OF_MEMORY_IN_BLOCKS;
    }

    void debugHeap();
};

class Code : public Memory
{
  public:
    Code(int SIZE_OF_PROGRAMM);
    virtual ~Code();
};
