#include "demos/demos.h"

#include "contur/kernel.h"

#include <iostream>

void demoSynchronization(DemoContext &ctx)
{
    char ch;

    std::cout << '\n';
    std::cout << "create unsynchronized Process P1 and P2, execute, look Error:" << '\n';
    auto code_1 = std::make_shared<Code>(3);
    code_1->setCmd(Mov, r1, 23);
    code_1->setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
    code_1->setCmd(Int, 0, Exit);

    auto code_2 = std::make_shared<Code>(6);
    code_2->setCmd(Rmem, r3, 70); // чтение содержимого r1 из памяти по адресу 70
    code_2->setCmd(Mov, r2, 2);
    code_2->setCmd(Mul, r1, r2);  // регистр * регистр
    code_2->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
    code_2->setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
    code_2->setCmd(Int, 0, Exit);

    Handle *handle_1 = ctx.kernel().CreateProcess("P1", code_1); // создать процесс
    Handle *handle_2 = ctx.kernel().CreateProcess("P2", code_2); // создать процесс

    ctx.dispatcher().dispatch(); // изменяет состояние системы 2 вызова

    handle_1->ProcessTime();
    handle_2->ProcessTime();

    /* программы работают асинхронно, поэтому Р1 не успевает
    осуществить запись содержимого r1 в память по адресу 70,
    строка 2 программа Р2 читает содержимое r1 из памяти по адресу 70, но память пуста */
    std::cout << "Dump memory Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        ctx.memory().debugHeap();
    }

    ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
    ctx.kernel().TerminateProcess(*handle_2); // завершить процесс

    ctx.memory().clearMemory();

    std::cout << '\n';

    std::cout << "create synchronized Process P1 and P2" << '\n';

    std::cout << "using CriticalSection Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        handle_1 = ctx.kernel().CreateProcess("P1", code_1); // создать процесс
        handle_2 = ctx.kernel().CreateProcess("P2", code_2); // создать процесс

        ctx.kernel().EnterCriticalSection(*handle_1);
        ctx.kernel().EnterCriticalSection(*handle_2);

        // выполнение программы Р1
        std::cout << "----------P1 dispatch----------" << '\n';
        ctx.dispatcher().dispatch();               // изменяет состояние системы 1-й шаг
        ctx.kernel().DebugProcessImage(*handle_1); // состояние процессов
        ctx.kernel().DebugProcessImage(*handle_2); // состояние процессов
        ctx.kernel().LeaveCriticalSection(*handle_1);

        // выполнение программы Р2
        std::cout << "----------P2 dispatch----------" << '\n';
        ctx.dispatcher().dispatch();               // изменяет состояние системы 2-й шаг
        ctx.kernel().DebugProcessImage(*handle_2); // проверка: виртуальная память освобождена
        ctx.memory().debugHeap();
        ctx.kernel().LeaveCriticalSection(*handle_2);

        std::cout << "dispatch" << '\n';
        ctx.dispatcher().dispatch(); // изменяет состояние системы 3-й шаг
        handle_1->ProcessTime();
        handle_2->ProcessTime();

        ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
        ctx.kernel().TerminateProcess(*handle_2); // завершить процесс
    }

    ctx.memory().clearMemory();
}
