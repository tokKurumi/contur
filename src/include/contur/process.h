#pragma once

#include "contur/architecture_cpu.h"
#include "contur/handle.h"
#include <memory>
#include <string>

class Memory; // forward declaration

class Process : public Handle
{
  public:
    Process();
    virtual ~Process();

    std::string getUser() const
    {
        return user;
    }
    void setUser(const std::string &user)
    {
        this->user = user;
    }

    int getID() const
    {
        return Handle::getID();
    }
    void setID(int ID)
    {
        Handle::setID(ID);
    }

    void setState(State state)
    {
        this->state = state;
    }
    State getState() const
    {
        return state;
    }

  private:
    State state;

  protected:
    std::string user;
};

class PCB : public Process
{
  private:
    int addr;
    int virtualAddr;
    SysReg *sysReg;
    int timeSlice;
    int priority;

  public:
    PCB();
    virtual ~PCB();

    void setSysReg(SysReg &sysReg)
    {
        this->sysReg = &sysReg;
    }
    void resetSysReg()
    {
        this->sysReg = nullptr;
    }
    SysReg *getSysReg()
    {
        return sysReg;
    }

    int getAddr() const
    {
        return addr;
    }
    void setAddr(int addr)
    {
        this->addr = addr;
    }

    int getVirtualAddr() const
    {
        return virtualAddr;
    }
    void setVirtualAddr(int virtualAddr)
    {
        this->virtualAddr = virtualAddr;
    }

    int getPriority() const
    {
        return priority;
    }
    void setPriority(int priority)
    {
        this->priority = priority;
    }

    int getTimeSlice() const
    {
        return timeSlice;
    }
    void setTimeSlice(int timeSlice)
    {
        this->timeSlice = timeSlice;
    }

    std::string NameOf(State state);
};

class ProcessImage : public PCB
{
  private:
    std::shared_ptr<Memory> memory;
    bool status;
    int flag;

  protected:
    friend class Dispatcher;
    friend class Scheduler;

  public:
    ProcessImage();
    virtual ~ProcessImage();

    void Debug();
    void DebugTime();

    Memory *getCode()
    {
        return memory.get();
    }
    void setCode(std::shared_ptr<Memory> memory)
    {
        this->memory = std::move(memory);
    }

    void setStatus(bool status)
    {
        this->status = status;
    }
    bool getStatus() const
    {
        return status;
    }

    void setFlag(int flag)
    {
        this->flag = flag;
    }
    int getFlag() const
    {
        return flag;
    }
    void clearTime()
    {
        Handle::clearTime();
    }
};
