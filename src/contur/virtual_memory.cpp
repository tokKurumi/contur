#include "contur/virtual_memory.h"
#include <iostream>

VirtualMemory::VirtualMemory(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES)
    : SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES),
      currImage(0),
      image(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES)
{
}

void VirtualMemory::setAddrImage(int addr)
{
    currImage = addr;
}

ProcessImage &VirtualMemory::getImage()
{
    return image[currImage];
}

void VirtualMemory::setMemory(std::shared_ptr<Memory> memory)
{
    image[currImage].setStatus(false);
    image[currImage].setCode(std::move(memory));
}

std::shared_ptr<Memory> VirtualMemory::read(int addr)
{
    return image[addr].getCode();
}

int VirtualMemory::getAddrFreeImage()
{
    for (int i = 0; i < SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES; i++) {
        if (image[i].getStatus()) {
            image[i].setStatus(false);
            return i;
        }
    }
    return -1;
}

void VirtualMemory::clearImage(int addr)
{
    image[addr].setStatus(true);
    image[addr].setCode(nullptr);
    image[addr].setAddr(-1);
    image[addr].setID(-1);
    image[addr].setPriority(-1);
    image[addr].setTimeSlice(-1);
    image[addr].setState(NotRunning);
    image[addr].resetSysReg();
    image[addr].setUser("");
    image[addr].setVirtualAddr(-1);
    image[addr].clearTime();
    image[addr].setTimeSlice(-1);
}

void VirtualMemory::clearMemory()
{
    for (int i = 0; i < SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES; i++) {
        clearImage(i);
    }
    currImage = 0;
}

void VirtualMemory::Debug()
{
    for (int i = 0; i < SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES; i++) {
        std::cout << " VertualAddress = " << i << " status = " << image[i].getStatus() << " ID = " << image[i].getID()
                  << '\n';
    }
}
