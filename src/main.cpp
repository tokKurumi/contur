#include <iostream>
#include <string>
#include <valarray>

#include "contur/cpu.h"
#include "contur/dispatcher.h"
#include "contur/handle.h"
#include "contur/kernel.h"
#include "contur/memory.h"
#include "contur/mmu.h"
#include "contur/mp_dispatcher.h"
#include "contur/process.h"
#include "contur/scheduler.h"

int main()
{
    int SIZE_OF_MEMORY_IN_BLOCKS = 100; // размер оперативной памяти в блоках
    int MAX_PROCESS = 3;                // максимальное количество процессов

    Memory memory(SIZE_OF_MEMORY_IN_BLOCKS);
    MMU mmu(memory);

    auto cpu = std::make_unique<CPU>(memory, MAX_PROCESS, std::make_unique<LAN>(), std::make_unique<Device>());
    Interrupt interrupt = OK;

    // планирование расписания, выполнение программ для однопроцессорных систем
    Scheduler scheduler(*cpu, MAX_PROCESS);
    auto job = std::make_unique<Job[]>(MAX_PROCESS); // моделирование многозадачного режима 3
    char ch = 'Y';                                   /* демонстрация создания образа процесса	при создании объектов
                                            класса Code указывается точный размер кода программы в блоках, т. е. количество строк */
    std::shared_ptr<Code> code, code_1, code_2;
    Handle *handle_1, *handle_2;                        //	для двух процессов
    int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES = MAX_PROCESS; // размер виртуальной памяти в образах процесса
    Dispatcher dispatcher(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, scheduler, mmu);
    Kernel kernel(dispatcher);

    while (ch == 'Y' || ch == 'N') {
        std::cout << "\ndialog mode:\n"
                     "1. Computer architecture. execute program 1,2.................enter 1\n"
                     "2. OS architecture. MultiTasking execute programm 1 and 2.....enter 2\n"
                     "3. Model process. Create Process for programm 1 and 2.........enter 3\n"
                     "4. Synchronize process. CriticalSection for programm 1 and 2..enter 4\n"
                     "5. Main memory and Management Memory Unit.....................enter 5\n"
                     "6. Virtual memory and process control block - PCB.............enter 6\n"
                     "7. Scheduling for one processor system. Methods...............enter 7\n"
                     "8. MultiProcessor scheduling. Methods.........................enter 8"
                  << '\n';
        std::cin >> ch;

        switch (ch) {
        case '1':
            std::cout << "execute programm 1: Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                /* Programm 1: вычисление арифметического выражения
                (6*7 + 3) / 5 - 4 и вывод результата на устройство (Dev) печати
                Деление и умножение в модели CPU определены только между регистрами.
                Программа в начале работы размещается по нулевому адресу */
                memory.setCmd(Mov, r1, 6); // Перемещение содержимого второго операнда в первый (регистр)
                std::cout << '\n';
                std::cout << "dump SysReg & first block execution: Y/N-any" << '\n';
                std::cin >> ch;

                if (ch == 'Y') {
                    /* присвоить адрес в PC выполняемого блока;
                    для программы по умолчанию выделен пул регистров с индесом 0 */
                    scheduler.getSysReg(0).setState(PC, 0);
                    scheduler.DebugBlock(0, *cpu); /* трассировка состояния регистров и блока, его выполнение */
                }

                memory.setCmd(Mov, r2, 7);
                memory.setCmd(Mul, r1, r2); // регистр * регистр
                memory.setCmd(Add, r1, 3);
                memory.setCmd(Mov, r2, 5);
                memory.setCmd(Div, r1, r2);
                memory.setCmd(Sub, r1, 4);
                memory.setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
                memory.setCmd(Rmem, r3, 69); // чтение содержимого r3 из памяти по адресу 69
                memory.setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
                memory.setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
                memory.setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit
                std::cout << '\n';
                std::cout << " programm 1 in memory: dump memory Y/N-any" << '\n';
                std::cin >> ch;
                if (ch == 'Y') {
                    memory.debugHeap(); // просмотреть содержимое памяти после загрузки программы
                }
                std::cout << "dump SysReg & execute programm 1: Y/N-any" << '\n';
                std::cin >> ch;
                if (ch == 'Y') {
                    std::cout << "begin P1 cpu.execute" << '\n';
                    scheduler.DebugSysReg(0); // трассировка состояния регистров до выполнения
                    // выполнение программы, расположенной по адресу 0 идентификатор процесса id = 0 для Р1
                    interrupt = scheduler.execute(0, 0, *cpu);
                    scheduler.DebugSysReg(0); // трассировка состояния регистров после выполнения
                    std::cout << "end P1 cpu.execute interrupt = " << interrupt << '\n';
                    std::cout << '\n';
                }

                std::cout << "clear memory & dump: Y/N-any" << '\n';
                std::cin >> ch;
                if (ch == 'Y') {
                    memory.clearMemory();
                    memory.debugHeap();
                    scheduler.getSysReg(0).clearSysReg(); // очистить регистры
                }
            } // end execute programm 1

            std::cout << " execute programm 2: Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                /* Programm 2: умножение двух чисел 9*12 + 2
                mmu.setAlloc(O); — запись на место программы в памяти,
                память не очищается, запись производится в свободную память */
                mmu.setAlloc(20);
                memory.setCmd(Mov, r1, 9);
                memory.setCmd(Mov, r2, 12);
                memory.setCmd(Mul, r1, r2); //	регистр * регистр
                memory.setCmd(Add, r1, 2);
                memory.setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
                memory.setCmd(Rmem, r3, 70); // чтение содержимого r3 из памяти по адресу 70
                memory.setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
                memory.setCmd(Int, r3, Lan);
                memory.setCmd(Int, 0, Exit);
                std::cout << '\n';
                std::cout << "programm 2 in memory: dump memory Y/-any" << '\n';
                std::cin >> ch;
                if (ch == 'Y') {
                    memory.debugHeap(); // просмотреть содержимое памяти после загрузки программы
                }
                std::cout << " SysReg & execute programm 2: Y/-any" << '\n';
                std::cin >> ch;

                if (ch == 'Y') {
                    std::cout << "begin P2 cpu.execute" << '\n';
                    scheduler.DebugSysReg(1); //	трассировка состояния регистров
                    /* выполнение программы, расположенной по адресу 20
                    идентификатор процесса id = 1 для Р2 */
                    interrupt = scheduler.execute(20, 1, *cpu);
                    scheduler.DebugSysReg(1); // трассировка состояния регистров
                    std::cout << "end P2 cpu.execute interrupt = " << interrupt << '\n';
                    std::cout << '\n';
                }
                std::cout << "clear memory & execute programm 2 - Error: Y/N-any" << '\n';
                std::cin >> ch;
                if (ch == 'Y') {
                    memory.clearMemory();
                    std::cout << " begin P2 cpu.execute " << '\n';
                    interrupt = scheduler.execute(20, 1, *cpu);
                    std::cout << " end P2 cpu.execute interrupt =" << interrupt << '\n';
                    std::cout << '\n';
                    scheduler.getSysReg(1).clearSysReg(); // очистить регистры
                }
            } // end execute programm 2
            break;

        case '2':
            std::cout << '\n';
            std::cout << "clear memory & load P1: addr=10 & P2 addr=40" << '\n';
            memory.clearMemory();
            //	6 + 24 / 3 = 14
            mmu.setAlloc(10);
            memory.setCmd(Mov, r1, 24); // перемещение содержимого второго операнда в первый (регистр)
            memory.setCmd(Mov, r2, 3);
            memory.setCmd(Div, r1, r2); // регистр / регистр
            memory.setCmd(Add, r1, 6);
            memory.setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
            memory.setCmd(Rmem, r3, 69); // чтение содержимого r3 из памяти по адресу 69
            memory.setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
            memory.setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
            memory.setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit
            // 4 * 7 + (12 + 3) / 5 = 30
            mmu.setAlloc(40);
            memory.setCmd(Mov, r1, 4);
            memory.setCmd(Mov, r2, 7);
            memory.setCmd(Mul, r1, r2); // регистр + регистр
            memory.setCmd(Wmem, r1, 70);
            memory.setCmd(Rmem, r3, 70);
            memory.setCmd(Mov, r1, 12);
            memory.setCmd(Mov, r2, 3);
            memory.setCmd(Add, r1, r2);
            memory.setCmd(Mov, r2, 5);
            memory.setCmd(Div, r1, r2);
            memory.setCmd(Add, r1, 3);
            memory.setCmd(Mov, r1, 31);
            memory.setCmd(Wmem, r1, 71);
            memory.setCmd(Rmem, r3, 71); // чтение содержимого r3 из памяти по адресу 71
            memory.setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
            memory.setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
            memory.setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit

            std::cout << "dump memory: Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                memory.debugHeap();
            }

            job[0].set(10, 0, true);
            job[1].set(40, 1, true);

            std::cout << "Multitasking execute p1, p2: Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                std::cout << "begin P1, p2 cpu.execute" << '\n';
                scheduler.setSliceCPU(1);
                interrupt = scheduler.execute(job.get(), mmu);
                std::cout << "end P1, P2 cpu.execute interrupt = " << interrupt << '\n';
            }

            std::cout << "dump memory: Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                memory.debugHeap();
            }

            std::cout << "clear memory: Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                memory.clearMemory();
                memory.debugHeap();
                scheduler.getSysReg(1).clearSysReg();
            }
            break;

        case '3':
            std::cout << '\n';
            std::cout << "create Process P1 and P2 from code:" << '\n';
            /* 1. для каждой программы должен создаваться свой объект
            2. код в примерах записывается с 0 адреса */

            //(6 * 7 + 3) / 5 – 4 = 5
            code = std::make_shared<Code>(12);
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

            handle_1 = kernel.CreateProcess("P1", code); //	создать процесс

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

            handle_2 = kernel.CreateProcess("P2", code); // создать процесс
            kernel.DebugProcessImage(*handle_2);         // последний

            std::cout << "dump queue NotRunning Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                dispatcher.DebugQueue(NotRunning);
            }

            std::cout << "dispatcher queue & execute Y/N-any" << '\n';
            std::cin >> ch;

            dispatcher.dispatch(); /* изменяет состояние системы,
                   т. к. ядро не под управлением CPU для модели из трех очередей 2 вызова */

            std::cout << "dump memory: Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                memory.debugHeap();
            }

            std::cout << "dump queue Running Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                dispatcher.DebugQueue(Running);
            }

            std::cout << "dispatch queue & dump queue ExitProcess Y/N-any" << '\n';
            std::cin >> ch;

            dispatcher.dispatch(); // изменяет состояние системы

            if (ch == 'Y') {
                dispatcher.DebugQueue(ExitProcess);
            }

            std::cout << "TerminateProcess & dump ProcessImage Y/N-any" << '\n';
            std::cin >> ch;

            kernel.TerminateProcess(*handle_1); //	завершить процесс
            kernel.TerminateProcess(*handle_2); // завершить процесс

            if (ch == 'Y') {
                kernel.DebugProcessImage(*handle_1); //	проверка; виртуальная
                kernel.DebugProcessImage(*handle_2); // память освобождена
            }

            memory.clearMemory();
            break;

        case '4':
            std::cout << '\n';
            std::cout << "create unsynchronized Process P1 and P2, execute, look Error:" << '\n';
            code_1 = std::make_shared<Code>(3);
            code_1->setCmd(Mov, r1, 23);
            code_1->setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
            code_1->setCmd(Int, 0, Exit);

            code_2 = std::make_shared<Code>(6);
            code_2->setCmd(Rmem, r3, 70); // чтение содержимого r1 из памяти по адресу 70
            code_2->setCmd(Mov, r2, 2);
            code_2->setCmd(Mul, r1, r2);  // регистр * регистр
            code_2->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
            code_2->setCmd(Int, r3, Lan); // посыл содержимого r3 по сети
            code_2->setCmd(Int, 0, Exit);

            handle_1 = kernel.CreateProcess("P1", code_1); // создать процесс
            handle_2 = kernel.CreateProcess("P2", code_2); //	создать процесс

            dispatcher.dispatch(); // изменяет состояние системы 2 вызова

            handle_1->ProcessTime();
            handle_2->ProcessTime();

            /* программы работают асинхронно, поэтому Р1 не успевает
            осуществить запись содержимого r1 в память по адресу 70,
            строка 2 программа Р2 читает содержимое r1 из памяти по адресу 70, но память пуста */
            std::cout << "Dump memory Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                memory.debugHeap();
            }

            kernel.TerminateProcess(*handle_1); // завершить процесс
            kernel.TerminateProcess(*handle_2); // завершить процесс

            memory.clearMemory();

            std::cout << '\n';

            std::cout << "create synchronized Process P1 and P2" << '\n';

            std::cout << "using CriticalSection Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                handle_1 = kernel.CreateProcess("P1", code_1); // создать процесс
                handle_2 = kernel.CreateProcess("P2", code_2); // создать процесс

                kernel.EnterCriticalSection(*handle_1);
                kernel.EnterCriticalSection(*handle_2);

                // выполнение программы Р1
                std::cout << "----------P1 dispatch----------" << '\n';
                dispatcher.dispatch();               // изменяет состояние системы 1-й шаг
                kernel.DebugProcessImage(*handle_1); //	состояние процессов
                kernel.DebugProcessImage(*handle_2); //	состояние процессов
                kernel.LeaveCriticalSection(*handle_1);

                // выпополнение программы Р2
                std::cout << "----------P2 dispatch----------" << '\n';
                dispatcher.dispatch();               // изменяет состояние системы 2-й шаг
                kernel.DebugProcessImage(*handle_2); // проверка: виртуальная память освобождена
                memory.debugHeap();
                kernel.LeaveCriticalSection(*handle_2);

                std::cout << "dispatch" << '\n';
                dispatcher.dispatch(); // изменяет состояние системы 3-й шаг
                handle_1->ProcessTime();
                handle_2->ProcessTime();

                kernel.TerminateProcess(*handle_1); // завершить процесс
                kernel.TerminateProcess(*handle_2); // завершить процесс
            }

            memory.clearMemory();
            break;

        case '5':
            std::cout << '\n';
            std::cout << "Management Memory Unit dump Y/N-any" << '\n';
            /* пример программы показывает выполнение основных
            функций MMU — Management Memory Unit */
            code_1 = std::make_shared<Code>(3);
            code_1->setCmd(Mov, r1, 23);
            code_1->setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
            code_1->setCmd(Int, 0, Exit);

            std::cin >> ch;
            if (ch == 'Y') {
                std::cout << "1.Create process using Management Memory Unit" << '\n';
                handle_1 = kernel.CreateProcess("P1", code_1); //	создать процесс
                mmu.Debug(*handle_1);                          // dump MMU — Management Memory Unit

                std::cout << "2. Execute process" << '\n';
                dispatcher.dispatch(); // изменяет состояние системы 2 вызова

                std::cout << "3. Terminate process" << '\n';
                kernel.TerminateProcess(*handle_1); //	завершить процесс
                // dump Terminate
                memory.clearMemory();
            }

            std::cout << "Main memory and MMU Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                std::cout << "1. Create process, load in main memory, execute" << '\n';
                handle_1 = kernel.CreateProcess("P1", code_1); //	создать процесс
                dispatcher.dispatch();                         // изменяет состояние системы 2 вызова
                memory.debugHeap(); // программа удаляется из памяти; данные расположенные по адресу 70
                dispatcher.dispatch();

                std::cout << "2. Terminate process" << '\n';
                kernel.TerminateProcess(*handle_1); // завершить процесс

                std::cout << "3. Clear all memory" << '\n';
                memory.clearMemory();
                memory.debugHeap();
            }
            break;

        case '6':
            std::cout << '\n';
            std::cout << "Virtual memory Y/N-any" << '\n';
            /* пример иллюстрирует выполнение основных функций
            Virtual memory и process control block — PCB, Process Image */
            code_1 = std::make_shared<Code>(3);
            code_1->setCmd(Mov, r1, 23);
            // запись содержимого r1 в память по адресу 70
            code_1->setCmd(Wmem, r1, 70);
            code_1->setCmd(Int, 0, Exit);
            std::cin >> ch;
            if (ch == 'Y') {
                // 1. состояние виртуальной памяти в образах
                std::cout << "1. Virtual Memory state" << '\n';
                dispatcher.DebugVirtualMemory(); // структура виртуальной памяти
                // 2. выделение виртуальной памяти в образах
                std::cout << "2. Create process" << '\n';
                handle_1 = kernel.CreateProcess("P1", code_1); // создать процесс
                dispatcher.DebugVirtualMemory();
                // изменяет состояние системы и выполнение программы
                std::cout << "3. Process execute" << '\n';
                dispatcher.dispatch();
                dispatcher.dispatch();
                std::cout << "4. Terminate process" << '\n';
                kernel.TerminateProcess(*handle_1); // завершить процесс
                // 3. освобождение виртуальной памяти в образах
                dispatcher.DebugVirtualMemory();
                // 4. очистить основную память
                memory.clearMemory();
            }

            std::cout << '\n';
            std::cout << "dump PCB and process image Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                std::cout << "1. Create process" << '\n';
                handle_1 = kernel.CreateProcess("P1", code_1); //	создать процесс
                // 1. состояние блока PCB
                dispatcher.DebugPCB(*handle_1);
                std::cout << "2. Process execute" << '\n';
                dispatcher.dispatch(); // изменяет состояние системы 2 вызова
                // 2. состояние блока PCB после выполнения программы.
                // см. состояние PC
                dispatcher.DebugPCB(*handle_1);
                dispatcher.dispatch();
                std::cout << "3. Terminate process" << '\n';
                kernel.TerminateProcess(*handle_1); // завершить процесс
                // 3. состояние блока PCB после удаления программы из системы
                dispatcher.DebugPCB(*handle_1);
                memory.clearMemory();
            }

            break;

        case '7':
            /* 1. для программы "Pl" и "P2" создаются объекты процессы
            сравнение методов планирования
            Tenter — Время входа
            Tbegin — Время начала
            Tservice — Время обслуживания в очередях
            Texec — Время выполнения
            Tterminate — Время завершения
            Tround = Tservice + Texec — Время оборота
            Tround/Tservice — Нормализованное время */

            code_1 = std::make_shared<Code>(12);
            // перемещение содержимого второго операнда в первый(регистр)
            code_1->setCmd(Mov, r1, 6);
            code_1->setCmd(Mov, r2, 7);
            code_1->setCmd(Mul, r1, r2); // регистр * регистр
            code_1->setCmd(Add, r1, 3);
            code_1->setCmd(Mov, r2, 5);
            code_1->setCmd(Div, r1, r2);
            code_1->setCmd(Sub, r1, 4);
            code_1->setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
            code_1->setCmd(Rmem, r3, 69);
            code_1->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
            code_1->setCmd(Int, r3, Lan); // посыл содержимого r1 по сети
            code_1->setCmd(Int, 0, Exit);

            code_2 = std::make_shared<Code>(9);
            code_2->setCmd(Mov, r1, 9);
            code_2->setCmd(Mov, r2, 12);
            code_2->setCmd(Mul, r1, r2);
            code_2->setCmd(Add, r1, 2);
            code_2->setCmd(Wmem, r1, 70);
            code_2->setCmd(Rmem, r3, 70);
            code_2->setCmd(Int, r1, Dev);
            code_2->setCmd(Int, r3, Lan);
            code_2->setCmd(Int, 0, Exit);

            std::cout << '\n';
            std::cout << "scheduler FCFS Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                Timer::setTime();                              // устанавливаем таймер в нулевое значение
                handle_1 = kernel.CreateProcess("P1", code_1); //	создать процесс
                handle_2 = kernel.CreateProcess("P2", code_2); // создать процесс
                dispatcher.dispatch(); // изменяет состояние системы для модели из трех очередей 2 вызова
                dispatcher.dispatch();
                handle_1->ProcessTime();
                handle_2->ProcessTime();
                kernel.TerminateProcess(*handle_1); // завершить процесс
                kernel.TerminateProcess(*handle_2); // завершить процесс
                /* очереди должны быть пусты после эксперимента
                scheduler.DebugSPNQueue(); */
                memory.clearMemory();
            }

            std::cout << '\n';
            std::cout << "scheduler RR Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                int slice = 5;
                Timer::setTime(); // устанавливаем таймер в нулевое значение
                // в зависимости от размера программы и квоты, изменяется число строк dispatcher.dispatch();
                dispatcher.setTimeSlice(slice);
                handle_1 = kernel.CreateProcess("P1", code_1); //	создать процесс
                handle_2 = kernel.CreateProcess("P2", code_2); //	создать процесс
                int disp = 4;                                  // подобрано по диаграмме slice и организации	очередей
                for (int i = 0; i < disp; i++) {
                    dispatcher.dispatch(); // изменяет состояние системы
                }

                handle_1->ProcessTime();
                handle_2->ProcessTime();
                kernel.TerminateProcess(*handle_1); // завершить процесс
                kernel.TerminateProcess(*handle_2); // завершить процесс
                memory.clearMemory();
                // очереди должны быть пусты после эксперимента scheduler.DebugSPNQueue();
                dispatcher.resetTimeSlice();
            }

            std::cout << '\n';
            std::cout << "scheduler SPN (shortest process next) Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                int numProcess =
                    5; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
                Timer::setTime(); // устанавливаем таймер в нулевое значение
                // включить регистрацию наблюдений
                dispatcher.setTpredict(TimeExecNotPreem);
                /* для невытесняющего метода
                alfa = 0.8; установлен взвешенный коэффициент 0 < alfa < 1
                в ShortestProcessNext(), открыт отладчик */
                for (int i = 0; i < numProcess; i++) {
                    std::cout << '\n';
                    std::cout << "--------------------experiment i = " << i << '\n';
                    handle_1 = kernel.CreateProcess("user_1", code_1);
                    handle_2 = kernel.CreateProcess("user_2", code_2);
                    dispatcher.dispatch(); // изменяет состояние системы
                    dispatcher.dispatch();
                    if (i < numProcess - 1) {
                        // -1, для сохранения информации о последнем эксперименте
                        kernel.TerminateProcess(*handle_1); //	завершить процесс
                        kernel.TerminateProcess(*handle_2); // завершить процесс
                    }

                    // наблюдения сохраняются, только после завершения процесса для первого процесса нет наблюдений
                    memory.clearMemory();
                }

                handle_1->ProcessTime();
                handle_2->ProcessTime();
                kernel.TerminateProcess(*handle_1); //	завершить процесс
                kernel.TerminateProcess(*handle_2); //	завершить процесс
                //	scheduler.DebugSPNQueue();
                dispatcher.resetTpredict(); // выключить регистрацию наблюдений
            }

            std::cout << '\n';
            std::cout << "scheduler SRT (shortest remaining time) Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                // представляет собой вытесняющую версию SPN
                int numProcess =
                    3; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
                Timer::setTime(); //	устанавливаем таймер в нулевое значение
                // включить регистрацию наблюдений процессов
                dispatcher.setTpredict(TimeExec);
                // и вытеснение
                for (int i = 0; i < numProcess; i++) {
                    std::cout << '\n';
                    std::cout << "----------------------experiment i = " << i << '\n';
                    // параметр 0 включает наименьшее оставшееся время процесса
                    handle_1 = kernel.CreateProcess("user_1", code_1);
                    handle_2 = kernel.CreateProcess("user_2", code_2);
                    dispatcher.dispatch(); // изменяет состояние системы
                    dispatcher.dispatch(); // вытеснение user_1 и выполнение
                    dispatcher.dispatch(); // длинный процесс не в swapOut, не успевает
                    if (i < numProcess - 1) {
                        // -1, для сохранения информации о последнем эксперименте
                        kernel.TerminateProcess(*handle_1); // завершить процесс
                        kernel.TerminateProcess(*handle_2); // завершить процесс
                    }
                    // наблюдения сохраняются, только после завершения процесса для первого экземпляра процесса нет
                    // наблюдений
                    memory.clearMemory();
                }

                handle_1->ProcessTime();
                handle_2->ProcessTime();
                kernel.TerminateProcess(*handle_1); // завершить процесс
                kernel.TerminateProcess(*handle_2); // завершить процесс
                //	scheduler. DebugSPNQueue();
                dispatcher.resetTpredict(); // выключить регистрацию наблюдений
            }

            std::cout << '\n';
            std::cout << "scheduler HRRN (hightst response ratio next) Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                // представляет собой вытесняющую версию SPN
                int numProcess =
                    3; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
                Timer::setTime(); // устанавливаем таймер в нулевое значение
                // включить регистрацию наблюдений процессов
                dispatcher.setTpredict(TimeServ);
                // в ShortestProcessNext(), открыт отладчик
                for (int i = 0; i < numProcess; i++) {
                    std::cout << '\n';
                    std::cout << "--------------------experiment i = " << i << '\n';
                    // параметр 0, включает наименьшее оставшееся время процесса
                    handle_1 = kernel.CreateProcess("user_1", code_1);
                    handle_2 = kernel.CreateProcess("user_2", code_2);
                    dispatcher.dispatch(); // изменяет состояние системы
                    dispatcher.dispatch(); // вытеснение user_1 и выполнение
                    dispatcher.dispatch(); // процесс swapOut
                    if (i < numProcess - 1) {
                        // -1, для сохранения информации о последнем эксперименте
                        kernel.TerminateProcess(*handle_1); // завершить процесс
                        kernel.TerminateProcess(*handle_2); // завершить процесс
                        // наблюдения сохраняются, только после завершения процесса для первого экземпляра процесса нет
                        // наблюдений
                    }

                    memory.clearMemory();
                }

                handle_1->ProcessTime();
                handle_2->ProcessTime();
                kernel.TerminateProcess(*handle_1); // завершить процесс
                kernel.TerminateProcess(*handle_2); //
                // scheduler.DebugSPNQueue();
                dispatcher.resetTpredict(); // выключить регистрацию наблюдений
            }

            std::cout << '\n';
            std::cout << "scheduler dynamic priority DP Y/N-any" << '\n';
            std::cin >> ch;
            if (ch == 'Y') {
                int slice = 5;
                // priority 0..3 индекс массива. TimeSlice — значение элемента в массиве
                auto prSlice = std::make_unique<int[]>(3);
                prSlice[0] = 5;
                prSlice[1] = 3;
                prSlice[2] = 2;
                Timer::setTime(); // устанавливаем таймер в нулевое значение
                // в зависимости от размера программы и TimeSlice изменяется число строк dispatcher.dispatch();
                dispatcher.setTimeSlice(3, std::move(prSlice));
                handle_1 = kernel.CreateProcess("P1", code_1); // создать процесс
                dispatcher.dispatch();                         // изменяет состояние системы
                handle_2 = kernel.CreateProcess("P2", code_2); // создать процесс
                int disp = 5;                                  // где
                for (int i = 0; i < disp; i++) {
                    dispatcher.dispatch(); // изменяет состояние системы
                }

                handle_1->ProcessTime();
                handle_2->ProcessTime();
                kernel.TerminateProcess(*handle_1); // завершить процесс
                kernel.TerminateProcess(*handle_2); // завершить процесс
                memory.clearMemory();
                //	scheduler.DebugSPNQueue();
                dispatcher.resetTimeSlice();
            }
            break;

        case '8':
            /* 1. сравнение методов планирования многопроцессорной
            системы для программ "Р1" и "Р2" создаются объекты процессы
            Tenter — Время входа
            Tbegin — Время начала
            Tservice — Время обслуживания в очередях
            Texec — Время выполнения
            Tterminate — Время завершения
            Tround = Tservice + Texec — Время оборота
            Tround/Tservice — Нормализованное время */
            {
                // block begin
                code = std::make_shared<Code>(9);
                code->setCmd(Mov, r1, 9);
                code->setCmd(Mov, r2, 12);
                code->setCmd(Mul, r1, r2);
                code->setCmd(Add, r1, 2);
                code->setCmd(Wmem, r1, 70);
                code->setCmd(Rmem, r3, 70);
                code->setCmd(Int, r1, Dev);
                code->setCmd(Int, r3, Lan);
                code->setCmd(Int, 0, Exit);

                std::cout << '\n';
                std::cout << "Assignment of Processes to Processors Y/N-any" << '\n';
                /* Multiprocessor scheduling (MP) — многопроцессорное
                планирование. Метод Global queue — планирование очередей */
                int MAX_PROCESSOR = 4; // максимальное количество процессоров
                Scheduler mpScheduler(*cpu, MAX_PROCESS);
                MPDispatcher mpDispatcher(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, mpScheduler, mmu, MAX_PROCESSOR);
                Kernel mpKernel(mpDispatcher);
                std::valarray<Handle *> vaHandle(MAX_PROCESS);
                std::cin >> ch;
                if (ch == 'Y') {
                    Timer::setTime(); // устанавливаем таймер в нулевое значение
                    // 1. Состояние процессоров
                    mpDispatcher.MPDebug();
                    for (int i = 0; i < MAX_PROCESS; i++) {
                        vaHandle[i] = mpKernel.CreateProcess("P", code); // создать процесс
                    }

                    mpDispatcher.dispatch(); // изменяет состояние системы
                    mpDispatcher.dispatch(); //	для модели из трех очередей 2 вызова
                    mpDispatcher.dispatch();
                    for (int i = 0; i < MAX_PROCESS; i++) {
                        vaHandle[i]->ProcessTime();              // создать процесс
                        mpKernel.TerminateProcess(*vaHandle[i]); // завершить процесс
                    }

                    memory.clearMemory();
                }
            } // block end
            break;

        default:;
        } // end case
    } // end while

    return 0;
}
