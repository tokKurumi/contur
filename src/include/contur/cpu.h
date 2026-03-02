#pragma once

#include "contur/architecture_cpu.h"
#include "contur/device.h"
#include "contur/lan.h"
#include "contur/memory.h"

#include <memory>

class CPU
{
  public:
    CPU(Memory &memory, int MAX_PROCESS, std::unique_ptr<LAN> lan, std::unique_ptr<Device> dev);
    ~CPU();

    Interrupt exeInstr(int addr, SysReg &sysreg);
    void Debug(SysReg &sysreg);

  private:
    Memory &memory;
    int MAX_PROCESS;
    std::unique_ptr<LAN> lan;
    std::unique_ptr<Device> dev;

    Block &fetch(SysReg &sysreg);

  protected:
    virtual Interrupt decode(Block &block, SysReg &sysreg);
};
