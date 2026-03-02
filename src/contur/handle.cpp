#include "contur/handle.h"
#include <iostream>

Handle::Handle() : ID(-1), Tenter(-1), Tbegin(-1), Tservice(0), Tterminate(0)
{
    Texec = rand() % (4 - 1 + 1) + 1;
}

Handle::~Handle()
{
    std::cout << " object Handle deleted" << '\n';
}

void Handle::ProcessTime()
{
    std::cout << "        ProcessTime ID =  " << ID << '\n';
    std::cout << "Tenter:\t" << Tenter << '\n';
    std::cout << "Tbegin:\t" << Tbegin << '\n';
    std::cout << "Tservice:\t" << Tservice << '\n';
    std::cout << "Tterminate:\t" << Tterminate << '\n';
    std::cout << "Texec:\t   " << Texec << '\n';
    std::cout << "Tround:\t" << getTround() << '\n';
    std::cout << "Tnorm:\t" << getTnorm() << '\n';
}

void Handle::clearTime()
{
    Tenter = -1;
    Tbegin = -1;
    Tservice = 0;
    Texec = 0;
    Tterminate = 0;
}

CS::CS() : cs(false)
{
}

CS::~CS()
{
    std::cout << " object CS deleted" << '\n';
}
