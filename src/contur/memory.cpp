#include "contur/memory.h"
#include <iostream>

// Memory

void Memory::clearBlock(int addr)
{
    heap[addr].setState(true);
    heap[addr].setCode(0);
    heap[addr].setNregister(0);
    heap[addr].setOperand(0);
}

Memory::Memory(int SIZE_OF_MEMORY_IN_BLOCKS)
    : currBlock(0), SIZE_OF_MEMORY_IN_BLOCKS(SIZE_OF_MEMORY_IN_BLOCKS), heap(SIZE_OF_MEMORY_IN_BLOCKS)
{
}

void Memory::setCmd(int cmd, int op1, int op2)
{
    int addr = currBlock++;
    if (addr == -1) {
        std::cout << "No free blocks" << '\n';
        return;
    }
    heap[addr].setState(false);
    heap[addr].setCode(cmd);
    heap[addr].setNregister(op1);
    heap[addr].setOperand(op2);
}

int Memory::getAddrFreeBlock()
{
    for (int i = 0; i < SIZE_OF_MEMORY_IN_BLOCKS; i++) {
        if (heap[i].getState()) {
            return i;
        }
    }
    return -1;
}

void Memory::clearMemory()
{
    for (int i = 0; i < SIZE_OF_MEMORY_IN_BLOCKS; i++) {
        clearBlock(i);
    }
    currBlock = 0;
}

void Memory::clearMemory(int AddrFrom, int AddrTo)
{
    for (int i = AddrFrom; i < SIZE_OF_MEMORY_IN_BLOCKS; i++) {
        if (i > AddrTo) {
            return;
        }
        clearBlock(i);
    }
}

void Memory::debugHeap()
{
    for (int i = 0; i < SIZE_OF_MEMORY_IN_BLOCKS; i++) {
        std::cout << "heap[" << i << "] = " << " state=" << heap[i].getState() << " code=" << heap[i].getCode()
                  << " nregister=" << heap[i].getNregister() << " operand=" << heap[i].getOperand() << '\n';
    }
}

// Code

Code::Code(int SIZE_OF_PROGRAMM) : Memory(SIZE_OF_PROGRAMM)
{
}

Code::~Code()
{
    std::cout << " object Code deleted" << '\n';
}
