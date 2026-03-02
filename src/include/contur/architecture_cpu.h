#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <string>

enum Instruction {
    Mov,
    Add,
    Sub,
    Mul,
    Div,
    And,
    Or,
    Xor,
    Shlu,
    Shru,
    Shls,
    Shrs,
    Cmp,
    Je,
    Jne,
    Jge,
    Jgt,
    Jle,
    Jlt,
    Int,
    Pushw,
    Pushc,
    Popw,
    Popc,
    Rmem,
    Wmem
};

enum Interrupt { OK = 0, Error = -1, Empty = 2, Exit = 16, Sys = 11, Div_0 = 3, Lan = 32, Dev = 254 };

class Block
{
  public:
    Block();
    void setState(bool state)
    {
        this->state = state;
    }
    bool getState() const
    {
        return state;
    }

    void setCode(int code)
    {
        this->code = code;
    }
    int getCode() const
    {
        return code;
    }

    void setNregister(int nregister)
    {
        this->nregister = nregister;
    }
    int getNregister() const
    {
        return nregister;
    }

    void setOperand(int operand)
    {
        this->operand = operand;
    }
    int getOperand() const
    {
        return operand;
    }

    void debugBlock();

  private:
    bool state;
    int8_t code;
    int8_t nregister;
    int operand;
};

constexpr int NUMBER_OF_REGISTERS = 16;
constexpr int SIZE_OF_REGISTER_NAMES = 4;
enum Name { r1, r2, r3, r4, r5, r6, r7, r8, r9, r10_, r11_, r12_, r13_, r14_, PC, SP };

inline Name operator++(Name &rs, int)
{
    return rs = static_cast<Name>(rs + 1);
}

class SysReg
{
  public:
    class Register
    {
      public:
        Register();
        Register(int numreg, int state, Name name);

        int getNumReg() const
        {
            return numreg;
        }
        void setNumReg(int numreg)
        {
            this->numreg = numreg;
        }
        int getState() const
        {
            return state;
        }
        void setState(int state)
        {
            this->state = state;
        }

        Name getName() const
        {
            return name;
        }
        void setName(Name name)
        {
            this->name = name;
        }

        std::string NameOf();

      private:
        int numreg;
        Name name;
        int state;
    };

    SysReg();

    int getState(Name name);
    void setState(int nregister, int operand);
    void clearSysReg();
    void Debug();

    bool getStatus() const
    {
        return status;
    }
    void setStatus(bool status)
    {
        this->status = status;
    }

  private:
    Register register_[NUMBER_OF_REGISTERS];
    bool status;
};

constexpr int NUMBER_OF_STATE = 7;
enum State { NotRunning = 0, Running = 1, Blocked = 2, Swapped = 3, ExitProcess = 4, New = 5, Ready = 6 };

class Timer
{
  public:
    Timer()
    {
        time = 0;
    }
    static void setTime()
    {
        time = 0;
    }
    static int getTime()
    {
        return time;
    }
    static void Tick()
    {
        time++;
    }

  private:
    static int time;
};

enum StatTime { TimeExecNotPreem = 0, TimeExec = 1, TimeServ = 2, noParam = -1 };

constexpr int NUMBER_OF_PARAM = 3;

class VectorParam
{
  public:
    VectorParam();

    void setState(StatTime time);
    bool getState(StatTime time) const;
    StatTime getStateTime() const;
    void resetState(StatTime time);
    void setTthreshold(StatTime time, double Tthreshold);
    double getTthreshold(StatTime time) const;
    void clearVectorParam();

  private:
    class Param
    {
      public:
        Param() : Tthreshold(-1), state(false)
        {
        }
        bool state;
        double Tthreshold;
    };
    Param param[NUMBER_OF_PARAM];
};
