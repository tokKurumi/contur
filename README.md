# Contur 2

Contur 2 is an educational OS kernel simulator written in C++20. It models core kernel subsystems such as processes, scheduling, memory management, synchronization, IPC, syscalls, and a simple file system.

## Execution Modes

- Interpreted mode: runs educational bytecode step by step.
- Native mode: manages real host processes through platform abstractions.

## What Is Included

- Modular architecture with interface-first design (DIP)
- Multiple scheduling and page replacement policies
- Dispatcher and multiprocessor dispatch support
- Tracing subsystem and terminal visualization components
- Unit and integration tests (GoogleTest)

## Build

```bash
bash src/build.sh debug src
```

## Run Tests

```bash
cd src
ctest --preset debug --output-on-failure
```

## Documentation

All project documentation is published at:
https://contur.yudashkin-dev.ru/
