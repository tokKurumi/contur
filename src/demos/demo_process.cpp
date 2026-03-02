#include "demos/demos.h"

#include "contur/kernel.h"

#include <iostream>

void demoProcess(DemoContext &ctx)
{
    char ch;

    std::cout << '\n';
    std::cout << "create Process P1 and P2 from code:" << '\n';
    /* 1. для каждой программы должен создаваться свой объект
    2. код в примерах записывается с 0 адреса */

    //(6 * 7 + 3) / 5 – 4 = 5
    auto code = std::make_shared<Code>(12);
    code->setCmd(Mov, r1, 6); // перемещение содержимого второго операнда в первый (регистр)
    code->setCmd(Mov, r2, 7);
    code->setCmd(Mul, r1, r2); // регистр * регистр
    code->setCmd(Add, r1, 3);
    code->setCmd(Mov, r2, 5);
    code->setCmd(Div, r1, r2);
    code->setCmd(Sub, r1, 4);
    code->setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
    code->setCmd(Rmem, r3, 69); // чтение содержимого r3 из памяти по адресу 69
    code->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
    code->setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
    code->setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit

    std::cout << "programm 1 in code: dump code Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        code->debugHeap(); // просмотреть содержимое памяти, в которой программа
    }

    Handle *handle_1 = ctx.kernel().CreateProcess("P1", code); // создать процесс

    // 9 * 12 + 2 = 110
    code = std::make_shared<Code>(9);
    code->setCmd(Mov, r1, 9);
    code->setCmd(Mov, r2, 12);
    code->setCmd(Mul, r1, r2); // регистр * регистр
    code->setCmd(Add, r1, 2);
    code->setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
    code->setCmd(Rmem, r3, 70); // чтение содержимого r3 из памяти по адресу 70
    code->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
    code->setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
    code->setCmd(Int, 0, Exit);

    Handle *handle_2 = ctx.kernel().CreateProcess("P2", code); // создать процесс
    ctx.kernel().DebugProcessImage(*handle_2);                 // последний

    std::cout << "dump queue NotRunning Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        ctx.dispatcher().DebugQueue(NotRunning);
    }

    std::cout << "dispatcher queue & execute Y/N-any" << '\n';
    std::cin >> ch;

    ctx.dispatcher().dispatch(); /* изменяет состояние системы,
           т. к. ядро не под управлением CPU для модели из трех очередей 2 вызова */

    std::cout << "dump memory: Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        ctx.memory().debugHeap();
    }

    std::cout << "dump queue Running Y/N-any" << '\n';
    std::cin >> ch;
    if (ch == 'Y') {
        ctx.dispatcher().DebugQueue(Running);
    }

    std::cout << "dispatch queue & dump queue ExitProcess Y/N-any" << '\n';
    std::cin >> ch;

    ctx.dispatcher().dispatch(); // изменяет состояние системы

    if (ch == 'Y') {
        ctx.dispatcher().DebugQueue(ExitProcess);
    }

    std::cout << "TerminateProcess & dump ProcessImage Y/N-any" << '\n';
    std::cin >> ch;

    ctx.kernel().TerminateProcess(*handle_1); // завершить процесс
    ctx.kernel().TerminateProcess(*handle_2); // завершить процесс

    if (ch == 'Y') {
        ctx.kernel().DebugProcessImage(*handle_1); // проверка; виртуальная
        ctx.kernel().DebugProcessImage(*handle_2); // память освобождена
    }

    ctx.memory().clearMemory();
}
