#include "contur/cpu.h"
#include <iostream>

CPU::CPU(Memory &memory, int MAX_PROCESS, std::unique_ptr<LAN> lan, std::unique_ptr<Device> dev)
    : memory(memory), MAX_PROCESS(MAX_PROCESS), lan(std::move(lan)), dev(std::move(dev))
{
}

CPU::~CPU()
{
}

Interrupt CPU::exeInstr(int addr, SysReg &sysreg)
{
    Interrupt interrupt;
    Block &block = fetch(sysreg);

    if (block.getState()) {
        return Error;
    }

    interrupt = decode(block, sysreg);

    switch (interrupt) {
    case OK:
        sysreg.setState(PC, memory.getNextAddr(sysreg.getState(PC)));
        break;
    case Exit:
        std::cout << "Exit" << '\n';
        return Exit;
    case Dev:
        dev->printData(sysreg.getState(static_cast<Name>(block.getNregister())));
        sysreg.setState(PC, memory.getNextAddr(sysreg.getState(PC)));
        break;
    case Lan:
        lan->sendData(sysreg.getState(static_cast<Name>(block.getNregister())));
        sysreg.setState(PC, memory.getNextAddr(sysreg.getState(PC)));
        break;
    default:
        return interrupt;
    }

    return OK;
}

void CPU::Debug(SysReg &sysreg)
{
    sysreg.Debug();
    Block &block = fetch(sysreg);
    block.debugBlock();
    decode(block, sysreg);
    sysreg.Debug();
}

Block &CPU::fetch(SysReg &sysreg)
{
    return memory.read(sysreg.getState(PC));
}

Interrupt CPU::decode(Block &block, SysReg &sysreg)
{
    std::cout << " opearand " << block.getOperand() << '\n';
    switch (block.getCode()) {
    case Mov:
        sysreg.setState(block.getNregister(), block.getOperand());
        break;
    case Add:
        sysreg.setState(
            static_cast<Name>(block.getNregister()),
            sysreg.getState(static_cast<Name>(block.getNregister())) + block.getOperand()
        );
        break;
    case Sub:
        sysreg.setState(
            static_cast<Name>(block.getNregister()),
            sysreg.getState(static_cast<Name>(block.getNregister())) - block.getOperand()
        );
        break;
    case Mul:
        sysreg.setState(
            static_cast<Name>(block.getNregister()),
            sysreg.getState(static_cast<Name>(block.getNregister())) *
                sysreg.getState(static_cast<Name>(block.getOperand()))
        );
        break;
    case Div:
        if (sysreg.getState(static_cast<Name>(block.getOperand())) == 0) {
            return Div_0;
        }
        sysreg.setState(
            static_cast<Name>(block.getNregister()),
            sysreg.getState(static_cast<Name>(block.getNregister())) /
                sysreg.getState(static_cast<Name>(block.getOperand()))
        );
        break;
    case Int:
        return static_cast<Interrupt>(block.getOperand());
        break;
    case Wmem: {
        Block &mblock = memory.read(block.getOperand());
        if (!mblock.getState()) {
            return Error;
        }
        mblock.setOperand(sysreg.getState(static_cast<Name>(block.getNregister())));
        mblock.setState(false);
        break;
    }
    case Rmem: {
        Block &mblock = memory.read(block.getOperand());
        if (mblock.getState()) {
            return Empty;
        }
        sysreg.setState(static_cast<Name>(block.getNregister()), mblock.getOperand());
        break;
    }
    default:
        return Error;
    }
    return OK;
}
