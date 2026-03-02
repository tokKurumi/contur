#include "contur/mmu.h"
#include <iostream>

MMU::MMU(Memory &memory) : memory(memory)
{
}

void MMU::swapIn(ProcessImage &processImage)
{
    auto code = processImage.getCode();
    int addrPCB = processImage.getAddr();
    int addrReal = getRealAddr();

    if (addrPCB == -1) {
        processImage.getSysReg()->setState(PC, addrReal);
    } else {
        int addrOffPC = processImage.getSysReg()->getState(PC) - addrPCB;
        processImage.getSysReg()->setState(PC, addrOffPC + addrReal);
    }

    processImage.setAddr(addrReal);
    setAlloc(addrReal);

    for (int i = 0; i < code->getSizeMemory(); i++) {
        Block &block = code->read(i);
        if (!block.getState()) {
            memory.setCmd(block.getCode(), block.getNregister(), block.getOperand());
        }
    }
}

void MMU::swapOut(ProcessImage &processImage)
{
    int AddrFrom = processImage.getAddr();
    int AddrTo = AddrFrom + processImage.getCode()->getSizeMemory() - 1;
    memory.clearMemory(AddrFrom, AddrTo);
}

int MMU::getRealAddr()
{
    return this->memory.getAddrFreeBlock();
}

void MMU::setAlloc(int addr)
{
    memory.setAddrBlock(addr);
}

void MMU::Debug(Handle &handle)
{
    ProcessImage &processImage = static_cast<ProcessImage &>(handle);
    auto code = processImage.getCode();

    std::cout << '\n';
    std::cout << "-get address of loading programm in memory & set PC" << '\n';

    int addrPCB = processImage.getAddr();
    std::cout << "addrPCB = " << addrPCB << " address of loading programm from PCB" << '\n';

    int addrReal = getRealAddr();
    std::cout << "addrReal= " << addrReal << " address of loading programm Real" << '\n';

    if (addrPCB == -1) {
        std::cout << " if address in PCB == -1 set addrReal PC = " << addrReal << '\n';
    } else {
        std::cout << "PC = " << processImage.getSysReg()->getState(PC) << " address in PC " << '\n';
        int addrOffPC = processImage.getSysReg()->getState(PC) - addrPCB;
        std::cout << "addrOffPC = PC - addrPCB = " << addrOffPC << " address off set" << '\n';
        std::cout << "PC = addrOffPC + addrReal = " << addrOffPC + addrReal << '\n';
    }

    std::cout << '\n';
    std::cout << "read block from process image code to memory" << '\n';
    std::cout << "size code = " << code->getSizeMemory() << '\n';
    for (int i = 0; i < code->getSizeMemory(); i++) {
        Block &block = code->read(i);
        if (!block.getState()) {
            block.debugBlock();
        }
    }
}

void MMU::DebugMemory()
{
    memory.debugHeap();
}
