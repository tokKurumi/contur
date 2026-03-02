#include "demos/demos.h"

#include "contur/mmu.h"
#include "contur/scheduler.h"

#include <iostream>

void demoMultitasking(DemoContext &ctx)
{
    char ch;
    Interrupt interrupt;

    std::cout << '\n';
    std::cout << "clear memory & load P1: addr=10 & P2 addr=40" << '\n';
    ctx.memory().clearMemory();
    // 6 + 24 / 3 = 14
    ctx.mmu().setAlloc(10);
    ctx.memory().setCmd(Mov, r1, 24); // перемещение содержимого второго операнда в первый (регистр)
    ctx.memory().setCmd(Mov, r2, 3);
    ctx.memory().setCmd(Div, r1, r2); // регистр / регистр
    ctx.memory().setCmd(Add, r1, 6);
    ctx.memory().setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
    ctx.memory().setCmd(Rmem, r3, 69); // чтение содержимого r3 из памяти по адресу 69
    ctx.memory().setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
    ctx.memory().setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
    ctx.memory().setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit
    // 4 * 7 + (12 + 3) / 5 = 30
    ctx.mmu().setAlloc(40);
    ctx.memory().setCmd(Mov, r1, 4);
    ctx.memory().setCmd(Mov, r2, 7);
    ctx.memory().setCmd(Mul, r1, r2); // регистр + регистр
    ctx.memory().setCmd(Wmem, r1, 70);
    ctx.memory().setCmd(Rmem, r3, 70);
    ctx.memory().setCmd(Mov, r1, 12);
    ctx.memory().setCmd(Mov, r2, 3);
    ctx.memory().setCmd(Add, r1, r2);
    ctx.memory().setCmd(Mov, r2, 5);
    ctx.memory().setCmd(Div, r1, r2);
    ctx.memory().setCmd(Add, r1, 3);
    ctx.memory().setCmd(Mov, r1, 31);
    ctx.memory().setCmd(Wmem, r1, 71);
    ctx.memory().setCmd(Rmem, r3, 71); // чтение содержимого r3 из памяти по адресу 71
    ctx.memory().setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
    ctx.memory().setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
    ctx.memory().setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit

    std::cout << "dump memory: Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        ctx.memory().debugHeap();
    }

    auto job = std::make_unique<Job[]>(ctx.maxProcess());
    job[0].set(10, 0, true);
    job[1].set(40, 1, true);

    std::cout << "Multitasking execute p1, p2: Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        std::cout << "begin P1, p2 cpu.execute" << '\n';
        ctx.scheduler().setSliceCPU(1);
        interrupt = ctx.scheduler().execute(job.get(), ctx.mmu());
        std::cout << "end P1, P2 cpu.execute interrupt = " << interrupt << '\n';
    }

    std::cout << "dump memory: Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        ctx.memory().debugHeap();
    }

    std::cout << "clear memory: Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        ctx.memory().clearMemory();
        ctx.memory().debugHeap();
        ctx.scheduler().getSysReg(1).clearSysReg();
    }
}
