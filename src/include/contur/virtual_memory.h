#pragma once

#include "contur/architecture_cpu.h"
#include "contur/memory.h"
#include "contur/process.h"
#include <memory>
#include <vector>

class VirtualMemory
{
  private:
    int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES;
    int currImage;
    std::vector<ProcessImage> image;

  public:
    VirtualMemory(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES);

    virtual void setAddrImage(int addr);
    ProcessImage &getImage();
    virtual void setMemory(std::shared_ptr<Memory> memory);
    virtual Memory *read(int addr);
    virtual int getAddrFreeImage();
    virtual void clearImage(int addr);
    virtual void clearMemory();
    virtual void Debug();
};
