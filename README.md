# C0 Virtual Machine (C0VM)

A complete implementation of a virtual machine for the C0 programming language.

## Overview

The C0 Virtual Machine is a stack-based virtual machine that executes C0 bytecode. C0 is a safe subset of C designed for teaching imperative programming concepts. This VM implementation supports the full C0 instruction set including arithmetic operations, control flow, function calls, memory management, and array operations.

## Features

- **Complete C0 Instruction Set**: Implements all standard C0 bytecode instructions
- **Stack-based Architecture**: Uses operand stacks for computation
- **Memory Management**: Safe memory allocation and access with null pointer checking
- **Function Calls**: Support for both static and native function invocation
- **Array Operations**: Dynamic array allocation and element access
- **Error Handling**: Runtime error detection for division by zero, memory violations, and assertion failures
- **Debug Mode**: Optional debug output for instruction tracing

## Architecture

### Core Components

- **Operand Stack**: Stack-based computation using C0 values
- **Local Variables**: Array of local variables for each function frame
- **Call Stack**: Manages function call frames and returns
- **Program Counter**: Tracks current instruction position
- **Memory Management**: Handles dynamic allocation and pointer operations

### Instruction Categories

1. **Stack Operations**: `POP`, `DUP`, `SWAP`
2. **Arithmetic**: `IADD`, `ISUB`, `IMUL`, `IDIV`, `IREM`, `IAND`, `IOR`, `IXOR`, `ISHL`, `ISHR`
3. **Constants**: `BIPUSH`, `ILDC`, `ALDC`, `ACONST_NULL`
4. **Local Variables**: `VLOAD`, `VSTORE`
5. **Control Flow**: `IF_CMPEQ`, `IF_CMPNE`, `IF_ICMPLT`, `IF_ICMPGE`, `IF_ICMPGT`, `IF_ICMPLE`, `GOTO`
6. **Functions**: `INVOKESTATIC`, `INVOKENATIVE`, `RETURN`
7. **Memory**: `NEW`, `IMLOAD`, `IMSTORE`, `AMLOAD`, `AMSTORE`, `CMLOAD`, `CMSTORE`, `AADDF`
8. **Arrays**: `NEWARRAY`, `ARRAYLENGTH`, `AADDS`
9. **Error Handling**: `ATHROW`, `ASSERT`

## Building

### Prerequisites

- GCC compiler with C99 support
- C0 compiler (`cc0`) installed and in PATH
- Standard C libraries

### Compilation

The project includes a Makefile with two build targets:

```bash
# Build both debug and release versions
make

# Build release version (optimized)
make c0vm

# Build debug version (with sanitizers and debug output)
make c0vmd

# Clean build artifacts
make clean
```

### Build Targets

- **c0vm**: Release version with optimizations
- **c0vmd**: Debug version with undefined behavior sanitizer and debug output

## Usage

### Compiling C0 Programs

First, compile your C0 source code to bytecode:

```bash
cc0 -b program.c0
```

This generates `program.bc0` bytecode file.

### Running Programs

Execute the bytecode using the VM:

```bash
# Release version
./c0vm program.bc0

# Debug version (with instruction tracing)
./c0vmd program.bc0
```

### Example

```bash
# Compile the test program
cc0 -b tests/iadd.c0

# Run with the VM
./c0vm tests/iadd.bc0
```

## Project Structure

```
c0vm/
├── README.md              # This file
├── README.txt             # Original course documentation
├── Makefile               # Build configuration
├── c0vm.c                 # Main VM implementation
├── c0vm_main.c            # Entry point and program loading
├── c0vm-ref.txt           # Bytecode instruction reference
├── lib/                   # Library files and headers
│   ├── *.h                # Header files for VM components
│   ├── *-safe.o           # Debug library objects
│   └── *-fast.o           # Release library objects
└── tests/                 # Test C0 programs
    ├── iadd.c0            # Simple arithmetic test
    ├── arrays.c0          # Array operations test
    ├── hellosir.c0        # String operations test
    └── ...                # Additional test cases
```

## Implementation Details

### Memory Safety

The VM implements several safety features:

- **Null Pointer Checking**: All memory accesses check for null pointers
- **Array Bounds Checking**: Array access validates indices are within bounds
- **Division by Zero**: Arithmetic operations check for division by zero
- **Integer Overflow**: Handles signed integer overflow cases

### Error Handling

The VM provides detailed error reporting for:

- **Arithmetic Errors**: Division by zero, invalid shifts
- **Memory Errors**: Null pointer dereference, invalid array access
- **Assertion Failures**: Failed assertions with custom messages
- **User Errors**: Explicit error throwing via `ATHROW`

### Debug Mode

When compiled with debug flags (`c0vmd`), the VM provides:

- Instruction-by-instruction tracing
- Stack size monitoring
- Program counter tracking
- Return value logging

## Testing

The `tests/` directory contains various C0 programs for testing different VM features:

- **iadd.c0**: Basic arithmetic operations
- **arrays.c0**: Array allocation and access
- **arith.c0**: Complex arithmetic expressions
- **hellosir.c0**: String operations
- **isqrt.c0**: Iterative algorithms

Run tests by compiling and executing:

```bash
cc0 -b tests/test_name.c0
./c0vm tests/test_name.bc0
```

## Development

### Adding New Instructions

To add support for new bytecode instructions:

1. Add the opcode case to the main switch statement in `c0vm.c`
2. Implement the instruction logic following the stack-based model
3. Update the program counter appropriately
4. Add error checking as needed

### Debugging

Use the debug version for development:

```bash
make c0vmd
./c0vmd program.bc0
```

This provides detailed execution traces showing:
- Current opcode being executed
- Stack size at each step
- Program counter position

## Course Context

This project is part of a imperative programming course, focusing on:

- **Imperative Programming**: Understanding program execution models
- **Memory Management**: Safe memory allocation and access patterns
- **Data Structures**: Stack-based computation and call frames
- **System Programming**: Low-level implementation details

## License

Copyright Carnegie Mellon University 2022.
