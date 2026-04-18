# NCDC Assembly Module Project — RISC-V Assembler

## Course
**NCDC Cohort 02/2025 — Design Verification (DV)**
NUST Chip Design Centre (NCDC), NUST

## Module
**RISC-V Assembly Module** — End-of-Module Project

---

## Overview

This project implements a **two-pass RISC-V assembler** written in C++. The assembler translates human-readable RISC-V assembly source code into 32-bit machine code (binary/hex format), supporting all major RISC-V base integer instruction set (RV32I) instruction types. The output is suitable for direct loading into a RISC-V processor simulation or hardware implementation.

---

## RISC-V Instruction Set Supported

The assembler handles all six RISC-V instruction encoding formats:

| Format | Instruction Examples | Description |
|--------|---------------------|-------------|
| **R-Type** | `add`, `sub`, `and`, `or`, `xor`, `sll`, `srl`, `sra` | Register-to-register arithmetic/logic |
| **I-Type** | `addi`, `lw`, `lh`, `lb`, `jalr`, `slli` | Immediate operand instructions |
| **S-Type** | `sw`, `sh`, `sb` | Store instructions |
| **B-Type** | `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu` | Branch instructions |
| **U-Type** | `lui`, `auipc` | Upper-immediate instructions |
| **J-Type** | `jal` | Jump-and-link |

---

## Repository Structure

```
ASsembler project/
├── assembler.cpp     # Main assembler source — full two-pass implementation
├── assembler          # Compiled assembler binary (Linux x86-64)
├── prog.s             # Sample RISC-V assembly program used for testing
└── out.hex            # Assembled machine code output in hexadecimal format
NCDC_Assembler_project_report.pdf   # Full project report with design and testing details
```

---

## Two-Pass Assembly Process

### Pass 1 — Symbol Table Construction
- Scan all lines of the assembly source file.
- Record the address of every label encountered (resolving forward references).
- Build a complete symbol table mapping label names to memory addresses.

### Pass 2 — Code Generation
- Re-scan all lines.
- For each instruction, look up operand values (including labels from the symbol table).
- Encode the instruction into its 32-bit binary representation according to the RISC-V ISA spec.
- Write the encoded word to the output file in hex format.

---

## How to Build and Run

```bash
# Compile the assembler
g++ -o assembler assembler.cpp -Wall -std=c++17

# Assemble a RISC-V program
./assembler prog.s out.hex

# View the output machine code
cat out.hex
```

### Sample Input (`prog.s`)
```asm
.text
main:
    addi t0, zero, 10     # t0 = 10
    addi t1, zero, 20     # t1 = 20
    add  t2, t0, t1       # t2 = t0 + t1
    sw   t2, 0(sp)        # store result
    beq  t0, t1, done     # branch if equal
    jal  zero, main       # loop
done:
    jalr zero, 0(ra)
```

---

## Concepts Demonstrated
- Lexical analysis and line-by-line parsing of assembly source
- Symbol table construction for label resolution
- Bitfield encoding of all RISC-V instruction formats (opcode, funct3, funct7, rs1, rs2, rd, imm)
- Two-pass assembler design to handle forward label references
- Hexadecimal machine code output compatible with RISC-V simulators
