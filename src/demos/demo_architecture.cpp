#include "demos/demos.h"

#include "contur/cpu.h"
#include "contur/mmu.h"
#include "contur/scheduler.h"

#include <iostream>

void demoArchitecture(DemoContext &ctx)
{
    char ch;
    Interrupt interrupt;

    std::cout << "execute programm 1: Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        /* Programm 1: вычисление арифметического выражения
        (6*7 + 3) / 5 - 4 и вывод результата на устройство (Dev) печати
        Деление и умножение в модели CPU определены только между регистрами.
        Программа в начале работы размещается по нулевому адресу */
        ctx.memory().setCmd(Mov, r1, 6); // Перемещение содержимого второго операнда в первый (регистр)
        std::cout << '\n';
        std::cout << "dump SysReg & first block execution: Y/N-any" << '\n';
        std::cin >> ch;

        if (ch == 'Y') {
            /* присвоить адрес в PC выполняемого блока;
            для программы по умолчанию выделен пул регистров с индесом 0 */
            ctx.scheduler().getSysReg(0).setState(PC, 0);
            ctx.scheduler().DebugBlock(0, ctx.cpu()); /* трассировка состояния регистров и блока, его выполнение */
        }

        ctx.memory().setCmd(Mov, r2, 7);
        ctx.memory().setCmd(Mul, r1, r2); // регистр * регистр
        ctx.memory().setCmd(Add, r1, 3);
        ctx.memory().setCmd(Mov, r2, 5);
        ctx.memory().setCmd(Div, r1, r2);
        ctx.memory().setCmd(Sub, r1, 4);
        ctx.memory().setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
        ctx.memory().setCmd(Rmem, r3, 69); // чтение содержимого r3 из памяти по адресу 69
        ctx.memory().setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
        ctx.memory().setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
        ctx.memory().setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit
        std::cout << '\n';
        std::cout << " programm 1 in memory: dump memory Y/N-any" << '\n';
        std::cin >> ch;
        if (ch == 'Y') {
            ctx.memory().debugHeap(); // просмотреть содержимое памяти после загрузки программы
        }
        std::cout << "dump SysReg & execute programm 1: Y/N-any" << '\n';
        std::cin >> ch;
        if (ch == 'Y') {
            std::cout << "begin P1 cpu.execute" << '\n';
            ctx.scheduler().DebugSysReg(0); // трассировка состояния регистров до выполнения
            // выполнение программы, расположенной по адресу 0 идентификатор процесса id = 0 для Р1
            interrupt = ctx.scheduler().execute(0, 0, ctx.cpu());
            ctx.scheduler().DebugSysReg(0); // трассировка состояния регистров после выполнения
            std::cout << "end P1 cpu.execute interrupt = " << interrupt << '\n';
            std::cout << '\n';
        }

        std::cout << "clear memory & dump: Y/N-any" << '\n';
        std::cin >> ch;
        if (ch == 'Y') {
            ctx.memory().clearMemory();
            ctx.memory().debugHeap();
            ctx.scheduler().getSysReg(0).clearSysReg(); // очистить регистры
        }
    } // end execute programm 1

    std::cout << " execute programm 2: Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        /* Programm 2: умножение двух чисел 9*12 + 2
        mmu.setAlloc(O); — запись на место программы в памяти,
        память не очищается, запись производится в свободную память */
        ctx.mmu().setAlloc(20);
        ctx.memory().setCmd(Mov, r1, 9);
        ctx.memory().setCmd(Mov, r2, 12);
        ctx.memory().setCmd(Mul, r1, r2); // регистр * регистр
        ctx.memory().setCmd(Add, r1, 2);
        ctx.memory().setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
        ctx.memory().setCmd(Rmem, r3, 70); // чтение содержимого r3 из памяти по адресу 70
        ctx.memory().setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
        ctx.memory().setCmd(Int, r3, Lan);
        ctx.memory().setCmd(Int, 0, Exit);
        std::cout << '\n';
        std::cout << "programm 2 in memory: dump memory Y/-any" << '\n';
        std::cin >> ch;
        if (ch == 'Y') {
            ctx.memory().debugHeap(); // просмотреть содержимое памяти после загрузки программы
        }
        std::cout << " SysReg & execute programm 2: Y/-any" << '\n';
        std::cin >> ch;

        if (ch == 'Y') {
            std::cout << "begin P2 cpu.execute" << '\n';
            ctx.scheduler().DebugSysReg(1); // трассировка состояния регистров
            /* выполнение программы, расположенной по адресу 20
            идентификатор процесса id = 1 для Р2 */
            interrupt = ctx.scheduler().execute(20, 1, ctx.cpu());
            ctx.scheduler().DebugSysReg(1); // трассировка состояния регистров
            std::cout << "end P2 cpu.execute interrupt = " << interrupt << '\n';
            std::cout << '\n';
        }
        std::cout << "clear memory & execute programm 2 - Error: Y/N-any" << '\n';
        std::cin >> ch;
        if (ch == 'Y') {
            ctx.memory().clearMemory();
            std::cout << " begin P2 cpu.execute " << '\n';
            interrupt = ctx.scheduler().execute(20, 1, ctx.cpu());
            std::cout << " end P2 cpu.execute interrupt =" << interrupt << '\n';
            std::cout << '\n';
            ctx.scheduler().getSysReg(1).clearSysReg(); // очистить регистры
        }
    } // end execute programm 2
}
