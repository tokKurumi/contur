#pragma once

#include "contur/memory.h"
#include "contur/process.h"

class MMU
{
  private:
    Memory &memory;

  public:
    MMU(Memory &memory);

    virtual void swapIn(ProcessImage &processImage);
    virtual void swapOut(ProcessImage &processImage);
    virtual int getRealAddr();
    virtual void setAlloc(int addr);
    virtual void Debug(Handle &handle);
    virtual void DebugMemory();
};
