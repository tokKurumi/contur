#include "contur/architecture_cpu.h"
#include <iostream>

// Block

Block::Block() : state(true), code(0), nregister(0), operand(0)
{
}

void Block::debugBlock()
{
    std::cout << " state=" << getState() << " code=" << getCode() << " nregister=" << getNregister()
              << " operand = " << getOperand() << '\n';
}

// SysReg::Register

SysReg::Register::Register() : numreg(0), name(r1), state(0)
{
}

SysReg::Register::Register(int numreg, int state, Name name) : numreg(numreg), state(state), name(name)
{
}

std::string SysReg::Register::NameOf()
{
    const char *Numbers[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14"};

    if (getName() <= 13) {
        return std::string("r") + Numbers[getName()];
    }

    switch (getName()) {
    case PC:
        return "PC";
    case SP:
        return "SP";
    default:
        return "";
    }
}

// SysReg

SysReg::SysReg() : status(true)
{
    int j = 0, i = 1;
    for (Name curName = r1; curName <= SP; curName++) {
        register_[j++] = Register(i++, -1, curName);
    }
}

int SysReg::getState(Name name)
{
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        if (register_[i].getName() == name) {
            return (register_[i].getState());
        }
    }
    return -1;
}

void SysReg::setState(int nregister, int operand)
{
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        if (register_[i].getName() == nregister) {
            register_[i].setState(operand);
            return;
        }
    }
}

void SysReg::clearSysReg()
{
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        register_[i].setState(-1);
    }
}

void SysReg::Debug()
{
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        std::cout << "register_[" << i << "] " << " numreg= " << register_[i].getNumReg()
                  << " state = " << register_[i].getState() << " name = " << register_[i].getName()
                  << " NameOf= " << register_[i].NameOf() << '\n';
    }
    std::cout << '\n';
}

// Timer

int Timer::time = 0;

// VectorParam

VectorParam::VectorParam()
{
    for (int i = 0; i < NUMBER_OF_PARAM; i++) {
        param[i] = Param();
    }
}

void VectorParam::setState(StatTime time)
{
    param[time].state = true;
}
bool VectorParam::getState(StatTime time) const
{
    return param[time].state;
}

StatTime VectorParam::getStateTime() const
{
    for (int i = 0; i < NUMBER_OF_PARAM; i++) {
        if (param[i].state) {
            if (i == 0) {
                return TimeExecNotPreem;
            }
            if (i == 1) {
                return TimeExec;
            }
            if (i == 2) {
                return TimeServ;
            }
        }
    }
    return noParam;
}

void VectorParam::resetState(StatTime time)
{
    param[time].state = false;
}
void VectorParam::setTthreshold(StatTime time, double Tthreshold)
{
    param[time].Tthreshold = Tthreshold;
}
double VectorParam::getTthreshold(StatTime time) const
{
    return param[time].Tthreshold;
}

void VectorParam::clearVectorParam()
{
    for (int i = 0; i < NUMBER_OF_PARAM; i++) {
        param[i].state = false;
        param[i].Tthreshold = -1;
    }
}
